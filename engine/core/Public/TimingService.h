#ifndef ARCHE_CORE_TIMINGSERVICE_H
#define ARCHE_CORE_TIMINGSERVICE_H

#include <chrono>

namespace Arche {
namespace Core {

/**
 * @brief TimingService provides high-resolution timing utilities for the engine.
 *        It can be used to measure frame times, elapsed time, and delta time.
 */
class TimingService {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    TimingService();

    /**
     * @brief Call at the start of each frame to update timing.
     */
    void tick();

    /**
     * @brief Returns the time elapsed since the last tick (in seconds).
     */
    float deltaTime() const;

    /**
     * @brief Returns the total time elapsed since the service was created (in seconds).
     */
    float elapsed() const;

    /**
     * @brief Resets the timer to the current time.
     */
    void reset();

private:
    TimePoint startTime_;
    TimePoint lastFrameTime_;
    float deltaTime_;
};

} // namespace Core
} // namespace Arche

#endif // ARCHE_CORE_TIMINGSERVICE_H