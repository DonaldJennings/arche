#ifndef I_GUISYSTEM_H
#define I_GUISYSTEM_H

#include <functional>
#include <memory>

#include "IPanel.h"
#include "UIContext.h"

namespace Arche {
    namespace GUI {

        class IGUISystem
        {
          public:
              virtual ~IGUISystem() = default;
              virtual void Startup() = 0;
              virtual void Shutdown() = 0;
              virtual void NewFrame() = 0;
              virtual void Render() = 0;
              virtual void SetDockController(std::function <void()> controller) = 0;
        };
    } // namespace GUI
} // namespace Arche

#endif // I_GUISYSTEM_H