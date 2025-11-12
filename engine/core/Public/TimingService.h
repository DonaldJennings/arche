#pragma once
#ifndef ARCHE_CORE_TIMINGSERVICE_H
#define ARCHE_CORE_TIMINGSERVICE_H

#include <LoggingService.h>
#include <chrono>

namespace Arche {
    namespace Core {

        /**
         * @brief TimingService provides high-resolution timing utilities for the engine.
         *        It can be used to measure frame times, elapsed time, and delta time.
         *
         * Behavior:
         *  - Call tick() once per frame (usually at frame start).
         *  - If paused, tick() produces deltaTime() == 0.0f.
         *  - pause()/resume() control paused state; resume() resets the internal last-frame time
         *    to avoid a large delta on resume.
         */
        class TimingService {
          public:
            using Clock = std::chrono::high_resolution_clock;
            using TimePoint = Clock::time_point;
            using Duration = std::chrono::duration<float>;

            TimingService(std::shared_ptr<Core::LoggingService> loggerIn)
                : logger_{loggerIn}, startTime_(Clock::now()), lastFrameTime_(startTime_), pauseStartTime_(),
                  pausedAccumulated_(Duration::zero()), deltaTime_(1.0/60.f), paused_(false) {};

            /**
             * @brief Call at the start of each frame to update timing.
             */
            void tick();

            /**
             * @brief Returns the time elapsed since the last tick (in seconds).
             *        Returns 0 when paused.
             */
            float deltaTime() const;

            /**
             * @brief Returns the total time elapsed since the service was created (in seconds),
             *        excluding time spent paused.
             */
            float elapsed() const;

            /**
             * @brief Resets the timer to the current time.
             */
            void reset();

            /**
             * @brief Pause timing. While paused, deltaTime() will be 0 and elapsed() will not advance.
             */
            void pause();

            /**
             * @brief Resume timing after pause.
             */
            void resume();

            /**
             * @brief Returns true when paused.
             */
            bool isPaused() const { return paused_; }

          private:
            std::shared_ptr<Arche::Core::LoggingService> logger_;
            TimePoint startTime_;
            TimePoint lastFrameTime_;
            TimePoint pauseStartTime_;
            Duration pausedAccumulated_;
            float deltaTime_;
            bool paused_;

        };

    } // namespace Core
} // namespace Arche

#endif // ARCHE_CORE_TIMINGSERVICE_H