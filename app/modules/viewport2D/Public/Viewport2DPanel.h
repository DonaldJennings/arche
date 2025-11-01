#pragma once

#include <IPanel.h>
#include <memory>
#include <string>
#include <string_view>
#include <UIContext.h>

namespace Arche {
    namespace GUI {
        class Viewport2DPanel : public IPanel {
          public:
            explicit Viewport2DPanel(std::shared_ptr<Arche::GUI::UIContext> panelContext);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<Arche::GUI::UIContext> context;

            void Reset();
        };
    } // namespace GUI
} // namespace Arche