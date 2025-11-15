#pragma once

#include "EditorSession.h"
#include <IPanel.h>
#include <memory>
#include <string>
#include <string_view>
#include <Theme.h>

namespace Arche {
    namespace GUI {
        class DockspacePanel : public IPanel {
          public:
            explicit DockspacePanel(std::shared_ptr<EditorSession> contextIn);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<EditorSession> context;
        };
    } // namespace GUI
} // namespace Arche