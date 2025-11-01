#include "LogPanel.h"

#include <imgui.h>
#include <LoggingService.h>
#include "GUILogSink.h"

namespace Arche {
    namespace GUI {

        LogPanel::LogPanel(GUILogSink *sink) : name("Log"), logSink(sink) {}

        std::string_view LogPanel::GetName() const { return name; }

        void LogPanel::Draw() {
            ImGui::Begin(name.c_str());
            if (logSink) {
                std::deque<LogMessage> messages = logSink->getLoggedMessages();
                for (const LogMessage &msg : messages) {
                    ImVec4 color;
                    switch (msg.level) {
                    case Arche::Core::LogLevel::INFO:
                        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
                        break;
                    case Arche::Core::LogLevel::WARNING:
                        color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                        break;
                    case Arche::Core::LogLevel::ERROR:
                        color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                        break;
                    default:
                        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white
                        break;
                    }

                    ImGui::TextColored(color, "%s", msg.text.c_str());
                }
            } else {
                ImGui::TextUnformatted("No log sink attached.");
            }
            ImGui::End();
        }

    } // namespace GUI
} // namespace Arche
