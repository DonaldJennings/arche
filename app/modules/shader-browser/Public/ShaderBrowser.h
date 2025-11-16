#pragma once

#include <string>
#include <memory>
#include <string_view>
#include <vector>
#include <chrono>
#include <cstdint>

#include <IPanel.h>
#include <WorldSystem.h>
#include "EditorSession.h"

namespace Arche {
    namespace GUI {

        class ShaderBrowser: public IPanel {
          public:
            explicit ShaderBrowser(std::shared_ptr<EditorSession> contextIn)
                : context{contextIn}, name{"Shader Browser"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:

            std::string name;
            std::shared_ptr<EditorSession> context;
        };

        
    class ShaderSourcePopup {
          public:
            ShaderSourcePopup(const std::string &popupId, const std::string &source) : id(popupId), text(source) {}

            void Open();

            void Draw();

            const std::string &getId() const { return id; }
          private:
            std::string id;
            std::string text;
        };

    } // namespace GUI
} // namespace Arche