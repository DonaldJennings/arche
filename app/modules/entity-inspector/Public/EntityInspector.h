#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "UIContext.h"
#include <IPanel.h>
#include <WorldSystem.h>

namespace Arche {
    namespace GUI {

        class EntityInspector : public IPanel {
          public:
            explicit EntityInspector(std::shared_ptr<UIContext> contextIn)
                : context{contextIn}, name{"Entity Inspector"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<UIContext> context;
        };

    } // namespace GUI
} // namespace Arche