#include "TimingService.h"
#include "LoggingService.h"

using namespace Arche::Core;

void TimingService::tick() {
    auto now = Clock::now();
    if (paused_) {
        // while paused, keep delta zero and don't advance lastFrameTime_
        deltaTime_ = 0.0f;
        return;
    }

    // normal advance
    Duration d = now - lastFrameTime_;
    deltaTime_ = d.count();
    lastFrameTime_ = now;
}

float TimingService::deltaTime() const {
    return deltaTime_;
}

float TimingService::elapsed() const {
    auto now = paused_ ? pauseStartTime_ : Clock::now();
    Duration total = now - startTime_ - pausedAccumulated_;
    return total.count();
}

void TimingService::reset() {
    startTime_ = Clock::now();
    lastFrameTime_ = startTime_;
    pausedAccumulated_ = Duration::zero();
    deltaTime_ = 0.0f;
    paused_ = false;
}

void TimingService::pause() {
    if (paused_) return;
    pauseStartTime_ = Clock::now();
    paused_ = true;
    ARCHE_LOG_INFO(logger_, "TimingService paused at elapsed time: " + std::to_string(elapsed()) + " seconds");
}

void TimingService::resume() {
    if (!paused_) return;
    auto now = Clock::now();
    pausedAccumulated_ += now - pauseStartTime_;
    // reset lastFrameTime_ so the next tick() doesn't produce a big delta
    lastFrameTime_ = now;
    paused_ = false;

    ARCHE_LOG_INFO(logger_, "TimingService resumed at elapsed time: " + std::to_string(elapsed()) + " seconds");
}