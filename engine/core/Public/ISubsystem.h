#ifndef ARCHE_CORE_ISUBSYSTEM_H
#define ARCHE_CORE_ISUBSYSTEM_H

namespace Arche {
    namespace Core {
        /**
         * @brief Interface for subsystems within the Arche engine.
         */
        class ISubsystem {
          public:
            // Virtual destructor
            virtual ~ISubsystem() = default;
            /**
             * @brief Initialize the subsystem.
             */
            virtual void initialise() = 0;

            /**
             * @brief Update the subsystem.
             * @param deltaTime Time elapsed since the last update.
             */
            virtual void update(double deltaTime) = 0;
            /**
             * @brief Shutdown the subsystem.
             */
            virtual void shutdown() = 0;
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_CORE_ISUBSYSTEM_H