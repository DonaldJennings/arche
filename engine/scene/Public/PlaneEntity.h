#pragma once
#include "PlaneCollider.h"
#include "IEntity.h"
#include <glm/glm.hpp>

namespace Arche {

    namespace Scene {
        class PlaneEntity : public IEntity {
          public:
            PlaneEntity(const glm::vec3 &normal, float distance)
                : m_Normal(glm::normalize(normal)), m_Distance(distance) {
                m_Position = m_Normal * distance;
                m_Rotation = glm::vec3(0.0f);
                m_Scale = glm::vec3(1.0f);
                m_Collider = std::make_shared<Physics::PlaneCollider>(m_Normal, m_Distance);
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
                // Static entity — no per-frame updates
            }

            // --- Components ---
            std::shared_ptr<Physics::RigidBody> getRigidBody() override { return m_Rigidbody; }
            std::shared_ptr<Physics::ICollider> getCollider() override { return m_Collider; }
            
            std::string_view getMeshId() override { return "plane.mesh"; }
            std::string_view getMaterialId() override { return "plane.mat"; }
            std::string_view getShaderId() override { return "flat.color"; }

            // --- Identification ---
            std::uint64_t getID() const override { return m_id; }

            void setID(std::uint64_t id) override { m_id = id; };

            std::string_view getName() const override { return m_name; }

            const glm::vec3 &GetNormal() const { return m_Normal; }
            float GetDistance() const { return m_Distance; }

          private:
            std::uint64_t m_id{};

            std::string m_name{"plane"};
            glm::vec3 m_Normal;
            float m_Distance;
            glm::vec3 m_Position;
            glm::vec3 m_Rotation;
            glm::vec3 m_Scale;

            std::shared_ptr<Physics::ICollider> m_Collider;
            std::shared_ptr<Render::IRenderable> m_Renderable;
            std::shared_ptr<Physics::RigidBody> m_Rigidbody;
        };
    } // namespace Scene
} // namespace Arche