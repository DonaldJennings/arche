
#pragma once

#include <IPanel.h>
#include <imgui.h>

#include "GUILogSink.h"

namespace Arche {
    namespace GUI {
        class LogPanel : public IPanel {
          public:
            LogPanel(GUILogSink* sink) : name("Log"), logSink{sink} {}
            virtual std::string_view GetName() const override { return name; }

            void Draw() override {
                ImGui::Begin(name.c_str());
                if (logSink) {
                    auto messages = logSink->getLoggedMessages();
                    for (const auto &msg : messages) {
                        ImGui::TextUnformatted(msg.c_str());
                    }
                } else {
                    ImGui::TextUnformatted("No log sink attached.");
                }
                ImGui::End();
            }

          private:
            std::string name;
            GUILogSink* logSink;
        };
    } // namespace GUI
} // namespace Arche
