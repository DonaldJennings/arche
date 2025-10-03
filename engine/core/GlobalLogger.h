#pragma once
#include "Diagnostics.h"
#include <memory>

namespace Arche {
    namespace Core {
        // Returns a shared global logger instance
        inline std::shared_ptr<Logger>& GetGlobalLogger() {
            static std::shared_ptr<Logger> logger = std::make_shared<Logger>();
            return logger;
        }
    }
}