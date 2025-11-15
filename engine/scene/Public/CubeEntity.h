#pragma once
#include "CubeCollider.h"
#include "ICollider.h"
#include "IEntity.h"
#include "Rigidbody.h"

namespace Arche {
    namespace Scene {
        class CubeEntity : public IEntity {
          public:
            CubeEntity(const glm::vec3 &halfExtents, const glm::vec3 &position, bool physicsEnabled = true)
                : m_Position(position), m_Rotation(0.0f), m_Scale(1.0f), m_HalfExtents(halfExtents),
                  m_Renderable{std::make_shared<Render::NamedRenderable>("cube.mesh", "cube.mat")} {
                if (physicsEnabled) {
                    m_Rigidbody = std::make_shared<Physics::RigidBody>(1.0f);
                    m_Collider = std::make_shared<Physics::CubeCollider>(halfExtents, &m_Position);
                }
            }

            // --- Transform ---
            glm::vec3 getPosition() const override { return m_Position; }
            glm::vec3 *getPositionPtr() override { return &m_Position; }
            void setPosition(const glm::vec3 &pos) override { m_Position = pos; }

            glm::vec3 getRotation() const override { return m_Rotation; }
            void setRotation(const glm::vec3 &rot) override { m_Rotation = rot; }

            glm::vec3 getScale() const override { return m_Scale; }
            void setScale(const glm::vec3 &scale) override { m_Scale = scale; }

            // --- Lifecycle ---
            void update(double deltaTime) override {
                // Optional cube-specific logic
            }

            // --- Components ---
            std::shared_ptr<Physics::RigidBody> getRigidBody() override { return m_Rigidbody; }
            std::shared_ptr<Physics::ICollider> getCollider() override { return m_Collider; }

            std::string_view getMeshId() override { return m_Renderable->getMeshName(); }
            std::string_view getMaterialId() override { return m_Renderable->getMaterialName(); }
            std::string_view getShaderId() override { return "flat.color"; }

            // --- Identification ---
            std::uint64_t getID() const override { return m_id; }

            void setID(std::uint64_t id) override { m_id = id; };

            std::string_view getName() const override { return m_name; }

            const glm::vec3 &GetHalfExtents() const { return m_HalfExtents; }

            std::shared_ptr<IEntity> clone() const override {
                auto cloned{std::make_shared<CubeEntity>(m_HalfExtents, m_Position)};
                cloned->setID(m_id);
                return cloned;
            }

          private:
            std::uint64_t m_id{};

            std::string m_name{"cube"};
            glm::vec3 m_Position;
            glm::vec3 m_Rotation;
            glm::vec3 m_Scale;
            glm::vec3 m_HalfExtents;

            std::shared_ptr<Physics::RigidBody> m_Rigidbody;
            std::shared_ptr<Physics::ICollider> m_Collider;
            std::shared_ptr<Render::NamedRenderable> m_Renderable;
        };
    } // namespace Scene
} // namespace Arche