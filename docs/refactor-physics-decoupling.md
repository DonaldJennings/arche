# Refactor: Physics Decoupling

Removes the raw `glm::vec3*` pointer from `PhysicsBody`, decoupling the physics system
from entity-owned memory. `PhysicsSystem` will own position state for each body; `WorldSystem`
syncs positions back to entities after each physics update.

## Current Data Flow (problem)

```
Entity owns m_position (glm::vec3)
    ↓ getPositionPtr() — raw pointer extracted
PhysicsBody stores glm::vec3* pointing INTO the entity
    ↓ integrate() writes back through the pointer
Entity position is mutated by physics behind the scene's back
```

## Target Data Flow

```
PhysicsSystem owns position for each body (by value)
    ↓ update() integrates, modifying its own data
WorldSystem reads positions back and calls entity->setPosition()
```

---

## Step 1 — Add `entityId` to `PhysicsBody`, change `position` to a value

**File:** `engine/physics/Public/PhysicsSystem.h`

```cpp
// Before
struct PhysicsBody {
    std::shared_ptr<RigidBody> rb;
    std::shared_ptr<ICollider> collider;
    glm::vec3 *position;
};

// After
struct PhysicsBody {
    std::uint64_t entityId{0};
    std::shared_ptr<RigidBody> rb;
    std::shared_ptr<ICollider> collider;
    glm::vec3 position{0.0f};
};
```

Also change `getBodies()` to return a const reference instead of a copy:

```cpp
inline const std::vector<PhysicsBody>& getBodies() const { return m_bodies; }
```

---

## Step 2 — Fix `PhysicsSystem::update()` to use the value member

**File:** `engine/physics/Public/PhysicsSystem.h` (`update()` method)

```cpp
// Before
body.rb->integrate(deltaTime, *body.position, gravity);

// After
body.rb->integrate(deltaTime, body.position, gravity);
```

`integrate()` takes `glm::vec3&` so it still writes back in-place — only the dereference changes.

---

## Step 3 — Update `WorldSystem::insertEntity` to copy position by value

**File:** `engine/scene/Private/WorldSystem.cpp`

```cpp
// Before
Physics::PhysicsBody body;
body.rb = entity->getRigidBody();
body.collider = entity->getCollider();
body.position = entity->getPositionPtr();

// After
Physics::PhysicsBody body;
body.entityId = assignedId;
body.rb = entity->getRigidBody();
body.collider = entity->getCollider();
body.position = entity->getPosition();
```

---

## Step 4 — Sync positions back after physics in `WorldSystem::update()`

**File:** `engine/scene/Public/WorldSystem.h` (`update()` method)

```cpp
void update(double dt) override {
    if (m_physicsSystem) {
        m_physicsSystem->update(dt);

        for (const auto& body : m_physicsSystem->getBodies()) {
            auto it = m_entityMap.find(body.entityId);
            if (it != m_entityMap.end()) {
                it->second->setPosition(body.position);
            }
        }
    }

    for (const auto &[id, entity] : m_entityMap) {
        entity->update(dt);
    }
}
```

**Run tests here** (`./build/bin/arche-engine-tests`) before continuing. Observable behaviour
is unchanged so existing physics tests should pass.

---

## Step 5 — Remove `getPositionPtr()` from `IEntity` and all implementations

**Files:** `engine/scene/Public/IEntity.h`, `SphereEntity.h`, `CubeEntity.h`, `PlaneEntity.h`

Delete from `IEntity.h`:
```cpp
virtual glm::vec3 *getPositionPtr() = 0;   // delete
```

Delete the override from each entity (all look like):
```cpp
glm::vec3 *getPositionPtr() override { return &m_position; }  // delete from each
```

---

## Step 6 — Fix the same raw pointer problem in the colliders

`SphereCollider` and `CubeCollider` both take `glm::vec3*` in their constructors and store it.
Make colliders pure shape descriptors — geometry only, no position pointer. The position will
be provided at query time when collision detection is implemented. Collision resolution is not
implemented yet so this removes unused coupling with no behaviour change.

**Files:** `engine/physics/Public/SphereCollider.h`, `engine/physics/Public/CubeCollider.h`

Remove the `glm::vec3*` constructor parameter and any stored pointer member from each collider.

Update the entity constructors accordingly:

```cpp
// SphereEntity — before
m_collider = std::make_shared<Physics::SphereCollider>(m_radius, &m_position);
// after
m_collider = std::make_shared<Physics::SphereCollider>(m_radius);

// CubeEntity — before
m_Collider = std::make_shared<Physics::CubeCollider>(halfExtents, &m_Position);
// after
m_Collider = std::make_shared<Physics::CubeCollider>(halfExtents);
```

---

## Step 7 — Fix `removeEntity` to also remove from `PhysicsSystem`

There is a documented bug: removing an entity from `WorldSystem` does not remove its physics
body from `PhysicsSystem`. Now that `PhysicsBody` has `entityId` this is straightforward.

**File:** `engine/physics/Public/PhysicsSystem.h` — add a new method:

```cpp
inline void removeBody(std::uint64_t entityId) {
    m_bodies.erase(
        std::remove_if(m_bodies.begin(), m_bodies.end(),
            [entityId](const PhysicsBody& b) { return b.entityId == entityId; }),
        m_bodies.end()
    );
}
```

**File:** `engine/scene/Public/WorldSystem.h` — update `removeEntity`:

```cpp
inline void removeEntity(std::uint64_t entityID) {
    m_entityMap.erase(entityID);
    if (m_physicsSystem) {
        m_physicsSystem->removeBody(entityID);
    }
}
```

---

## Order and Notes

- Steps 1–4 are a single logical unit — do them together and compile/test before moving on.
- Step 5 is cleanup that follows naturally once the raw pointer is gone.
- Steps 6 and 7 are independent and can be done in either order.
- After this refactor, `PhysicsSystem` no longer has any dependency on entity internals.
  The position data it operates on is fully owned, which is the prerequisite for GPU buffer
  upload in the GPGPU adoption path (see `gpgpu-adoption.md`).
