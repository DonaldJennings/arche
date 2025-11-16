#ifndef ARCHE_CORE_ISUBSYSTEM_H
#define ARCHE_CORE_ISUBSYSTEM_H

namespace Arche {
    namespace Core {
        /**
         * @brief Interface for subsystems within the Arche engine.
         * 
         * This abstract interface defines the lifecycle methods that all engine
         * subsystems must implement. Subsystems are major components of the engine
         * such as physics, rendering, scene management, etc. They follow a standard
         * initialization, update, and shutdown pattern.
         * 
         * @note All subsystem implementations must be thread-safe if they are to
         *       be used in multi-threaded contexts.
         */
        class ISubsystem {
          public:
            /**
             * @brief Virtual destructor for proper cleanup of derived classes.
             */
            virtual ~ISubsystem() = default;
            
            /**
             * @brief Initialize the subsystem.
             * 
             * This method is called once during engine startup to set up the subsystem.
             * Implementations should allocate resources, load configuration, and prepare
             * the subsystem for use.
             * 
             * @throws std::runtime_error if initialization fails
             */
            virtual void initialise() = 0;

            /**
             * @brief Update the subsystem for the current frame.
             * 
             * This method is called once per frame to update the subsystem's state.
             * The deltaTime parameter indicates how much time has passed since the
             * last update, allowing for frame-rate independent behavior.
             * 
             * @param deltaTime Time elapsed since the last update in seconds
             */
            virtual void update(double deltaTime) = 0;
            
            /**
             * @brief Shutdown the subsystem.
             * 
             * This method is called once during engine shutdown to clean up the subsystem.
             * Implementations should release all allocated resources and perform any
             * necessary cleanup operations.
             */
            virtual void shutdown() = 0;
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_CORE_ISUBSYSTEM_H