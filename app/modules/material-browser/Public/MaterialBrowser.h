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

        class MaterialBrowser : public IPanel {
          public:
            explicit MaterialBrowser(std::shared_ptr<EditorSession> contextIn)
                : context{contextIn}, name{"Material Browser"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:

            std::string name;
            std::shared_ptr<EditorSession> context;
        };

    } // namespace GUI
} // namespace Arche