#pragma once

#include <IPanel.h>
#include <imgui.h>

#include <LoggingService.h>
#include "GUILogSink.h"
#include <string>
#include <string_view>
#include <memory>

namespace Arche {
    namespace GUI {
        class LogPanel : public IPanel {
          public:
            explicit LogPanel(GUILogSink *sink);
            std::string_view GetName() const override;

            void Draw() override;

          private:
            std::string name;
            GUILogSink *logSink;
        };
    } // namespace GUI
} // namespace Arche
