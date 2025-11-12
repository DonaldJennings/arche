#pragma once

#include <string>
#include <memory>
#include <string_view>
#include <vector>
#include <chrono>
#include <cstdint>

#include <IPanel.h>
#include <WorldSystem.h>
#include "UIContext.h"

namespace Arche {
    namespace GUI {

        class WorldProperties : public IPanel {
          public:
            explicit WorldProperties(std::shared_ptr<UIContext> contextIn)
                : context{contextIn}, name{"World Properties"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:

            std::string name;
            std::shared_ptr<UIContext> context;
        };

    } // namespace GUI
} // namespace Arche