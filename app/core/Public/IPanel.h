#pragma once

#include <string>
#include <string_view>

namespace Arche {
    namespace GUI {
        class IPanel {
          public:
            virtual ~IPanel() = default;
            virtual void Draw() = 0;
            virtual std::string_view GetName() const = 0;
        };

    } // namespace GUI
} // namespace Arche