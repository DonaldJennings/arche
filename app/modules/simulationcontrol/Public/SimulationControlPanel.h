#pragma once

#include <IPanel.h>
#include <memory>
#include <string>
#include <string_view>

#include "UIContext.h"

namespace Arche {
    namespace GUI {
        class SimulationControlPanel : public IPanel {
          public:
            explicit SimulationControlPanel(std::shared_ptr<UIContext> contextIn);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<UIContext> context;
        };
    } // namespace GUI
} // namespace Arche
