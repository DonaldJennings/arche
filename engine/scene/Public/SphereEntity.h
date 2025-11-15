#pragma once
#include "IEntity.h"
#include "SphereCollider.h"
#include <memory>
#include <string>

namespace Arche {
    namespace Scene {
        class SphereEntity : public IEntity {
          public:
            SphereEntity(float radius, glm::vec3 position, bool physicsEnabled = true)
                : m_radius{radius}, m_position{position}, m_rotation{0.0f, 0.0f, 0.0f}, m_scale(radius) {
                // Initialize the collider as a sphere collider
                m_collider = std::make_shared<Physics::SphereCollider>(m_radius, &m_position);
                // If physics is enabled, set up the rigid body
                m_rigidBody = std::make_shared<Physics::RigidBody>(1.0f);
                if (physicsEnabled) {
                    m_rigidBody->setUseGravity(true);
                } else {
                    m_rigidBody->setStatic(true); // Make it static if physics is disabled
                }

                m_renderable = std::make_shared<Render::NamedRenderable>("uv_sphere.mesh", "uv_sphere.mat");
            }

            // Getters and setters
            glm::vec3 getPosition() const override { return m_position; };
            glm::vec3 *getPositionPtr() override { return &m_position; };
            void setPosition(const glm::vec3 &position) override { m_position = position; };
            glm::vec3 getRotation() const override { return m_rotation; };
            void setRotation(const glm::vec3 &rotation) override { m_rotation = rotation; };
            glm::vec3 getScale() const override { return m_scale; };
            void setScale(const glm::vec3 &scale) override { m_scale = scale; };

            // Updating entity state
            void update(double deltaTime) override {};

            // Getters for other systems
            std::shared_ptr<Physics::RigidBody> getRigidBody() override { return m_rigidBody; };
            std::shared_ptr<Physics::ICollider> getCollider() override { return m_collider; };

            std::string_view getMeshId() override { return m_renderable->getMeshName(); };
            std::string_view getMaterialId() override { return m_renderable->getMaterialName(); };
            void setMaterialId(std::string_view materialId) override {
                if (m_renderable) {
                    m_renderable->setMaterialName(std::string(materialId));
                }
            }
            std::string_view getShaderId() override { return m_renderable->getMaterialName(); };

            std::string_view getName() const override { return m_name; };
            std::uint64_t getID() const override { return m_id; };
            void setID(std::uint64_t id) override { m_id = id; };

            std::shared_ptr<IEntity> clone() const override {
                auto cloned{std::make_shared<SphereEntity>(m_radius, m_position)};
                cloned->setID(m_id);
                return cloned;
            }

          private:
            // Internal data members for position, rotation, scale, rigid body, collider, and renderable
            std::uint64_t m_id{};
            std::string m_name{"sphere"};
            glm::vec3 m_position;
            glm::vec3 m_rotation;
            glm::vec3 m_scale;
            float m_radius;

            std::shared_ptr<Physics::RigidBody> m_rigidBody;
            std::shared_ptr<Physics::ICollider> m_collider;
            std::shared_ptr<Render::NamedRenderable> m_renderable;
        };
    } // namespace Scene
} // namespace Arche
