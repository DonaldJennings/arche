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
            explicit WorldProperties(std::shared_ptr<UIContext> contextIn);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            void drawRendererConfiguration();
            void drawEditorSettings();
            void drawDirectionalLightSettings();
            void drawWorldConfiguration();
            void drawWorldInfo();
            void drawWorldVisuals();

            std::string name;
            std::shared_ptr<UIContext> context;
        };

    } // namespace GUI
} // namespace Arche