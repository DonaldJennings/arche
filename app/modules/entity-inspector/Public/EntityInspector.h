#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "EditorSession.h"
#include <IPanel.h>
#include <WorldSystem.h>

namespace Arche {
    namespace GUI {

        class EntityInspector : public IPanel {
          public:
            explicit EntityInspector(std::shared_ptr<EditorSession> contextIn)
                : context{contextIn}, name{"Entity Inspector"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<EditorSession> context;
        };

    } // namespace GUI
} // namespace Arche