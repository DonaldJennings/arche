#pragma once

#include <string>
#include <memory>
#include <string_view>

#include <IPanel.h>
#include <WorldSystem.h>
#include "UIContext.h"

namespace Arche {
    namespace GUI {
        class MetricsPanel : public IPanel {
          public:
            explicit MetricsPanel(std::shared_ptr<UIContext> contextIn);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<UIContext> context;
        };

    } // namespace GUI
} // namespace Arche