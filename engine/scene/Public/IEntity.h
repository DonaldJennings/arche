#pragma once

#include "ICollider.h"
#include "IRenderable.h"
#include "RigidBody.h"

#include <glm/glm.hpp>
#include <memory>
#include <string_view>

namespace Arche {
    namespace Scene {
        class IEntity {
          public:
            virtual ~IEntity() = default;

            virtual std::uint64_t getID() const = 0;
            
            virtual void setID(std::uint64_t) = 0;

            virtual glm::vec3 getPosition() const = 0;
            virtual glm::vec3 *getPositionPtr() = 0;
            virtual void setPosition(const glm::vec3 &position) = 0;

            virtual glm::vec3 getRotation() const = 0;
            virtual void setRotation(const glm::vec3 &rotation) = 0;

            virtual glm::vec3 getScale() const = 0;
            virtual void setScale(const glm::vec3 &scale) = 0;

            virtual void update(double deltaTime) = 0;

            virtual std::shared_ptr<Physics::RigidBody> getRigidBody() = 0;
            virtual std::shared_ptr<Physics::ICollider> getCollider() = 0;

            virtual std::string_view getMeshId() = 0;
            virtual std::string_view getMaterialId() = 0;
            virtual void setMaterialId(std::string_view materialId) = 0;
            virtual std::string_view getShaderId() = 0;
            
            virtual std::string_view getName() const = 0;
            
            virtual std::shared_ptr<IEntity> clone() const = 0;
        };
    } // namespace Scene
} // namespace Arche