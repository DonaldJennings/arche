#pragma once
#include <memory>
#include <string_view>
namespace Arche {
    namespace Render {

        class IRenderable {
          public:
            virtual ~IRenderable() = default;
            virtual std::string_view getMeshName() const = 0;
            virtual std::string_view getMaterialName() const = 0;
            virtual std::string_view getShaderName() const = 0;
        };

    } // namespace Scene
} // namespace Arche