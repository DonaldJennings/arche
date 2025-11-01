#include "TimingService.h"

namespace Arche {
namespace Core {

TimingService::TimingService()
    : startTime_(Clock::now()), lastFrameTime_(startTime_), deltaTime_(0.0f) {}

void TimingService::tick() {
    auto now = Clock::now();
    deltaTime_ = std::chrono::duration<float>(now - lastFrameTime_).count();
    lastFrameTime_ = now;
}

float TimingService::deltaTime() const {
    return deltaTime_;
}

float TimingService::elapsed() const {
    return std::chrono::duration<float>(Clock::now() - startTime_).count();
}

void TimingService::reset() {
    startTime_ = Clock::now();
    lastFrameTime_ = startTime_;
    deltaTime_ = 0.0f;
}

} // namespace Core
} // namespace Arche