#ifndef ARCHE_ENGINE_FIXED_TIMESTEP_CLOCK_H
#define ARCHE_ENGINE_FIXED_TIMESTEP_CLOCK_H

namespace Arche {
    namespace Core {

        /**
         * @brief A clock that manages fixed timestep updates, commonly used in
         * game loops to ensure consistent simulation steps regardless of frame
         * rate.
         */
        class FixedTimestepClock {
          public:
            /**
             * @brief Constructs a FixedTimestepClock with a specified fixed
             * timestep interval.
             * @param fixed_timestep The fixed time interval (in seconds) for
             * each tick. Defaults to 1/60th of a second.
             */
            explicit FixedTimestepClock(float fixed_timestep = 1.0f / 60.0f)
                : step_duration_(fixed_timestep) {}

            /**
             * @brief Adds the specified delta time to the accumulator.
             * @param delta_time The amount of time to add to the accumulator.
             */
            inline void accumulate(float delta_time) {
                accumulator_ += delta_time;
            }

            /**
             * @brief Checks if the accumulated time has reached or exceeded the
             * fixed timestep.
             * @return true if the accumulator is greater than or equal to the
             * fixed timestep; otherwise, false.
             */
            inline bool stepReady() const {
                constexpr float epsilon = 1e-6f;
                return accumulator_ >= step_duration_ - epsilon;
            }

            /**
             * @brief Consumes a simulation step if one is ready by decrementing
             * the accumulator.
             */
            void consumeStep() {
                if (stepReady()) {
                    accumulator_ -= step_duration_;

                    // Prevent negative accumulator due to floating-point
                    // precision issues
                    if (accumulator_ < 0.0f) {
                        accumulator_ = 0.0f;
                    }
                }
            }

            /**
             * @brief Calculates the interpolation factor based on the current
             * accumulator and fixed timestep values.
             * @return A floating-point value representing the interpolation
             * factor. Returns 0.0f if the fixed timestep is not positive.
             */
            float interpolationFactor() const {
                return step_duration_ > 0 ? accumulator_ / step_duration_
                                           : 0.0f;
            }

          private:
            float accumulator_ = 0.0f;
            float step_duration_ = 1.0f / 60.0f; // Default to 60 FPS
        };
    } // namespace Core
} // namespace Arche

#endif // ARCHE_ENGINE_FIXED_TIMESTEP_CLOCK_H