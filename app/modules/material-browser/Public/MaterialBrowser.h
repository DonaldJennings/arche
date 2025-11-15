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

        class MaterialBrowser : public IPanel {
          public:
            explicit MaterialBrowser(std::shared_ptr<UIContext> contextIn)
                : context{contextIn}, name{"Material Browser"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:

            std::string name;
            std::shared_ptr<UIContext> context;
        };

    } // namespace GUI
} // namespace Arche