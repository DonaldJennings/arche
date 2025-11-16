#pragma once

#include "LoggingService.h"
#include "TimingService.h"
#include "GlobalSettings.h"
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

// Forward declarations
namespace Arche {
    namespace Scene {
        class WorldSystem;
    }
    namespace Render {
        class RenderingSystem;
        class ShaderLoader;
        class MaterialLoader;
    }
    namespace Physics {
        class PhysicsSystem;
    }
} // namespace Arche

namespace Arche {
    namespace Core {

        /**
         * @brief Central hub of the Arche Engine, coordinating all major subsystems.
         * 
         * EngineCore is the main entry point and coordinator for the engine. It manages
         * the lifecycle of all subsystems including physics, rendering, scene management,
         * and provides simulation control (play, pause, reset). The engine follows a
         * strict initialization, update, and shutdown sequence.
         * 
         * @note This class uses shared_ptr for subsystem management to allow safe
         *       cross-system dependencies and avoid circular references.
         */
        class EngineCore {
          public:
            /**
             * @brief Enumeration of simulation states.
             * 
             * Defines the possible states of the physics simulation.
             */
            enum class SimulationState { 
                Idle,    ///< Simulation not started or has been reset
                Running, ///< Simulation actively updating
                Paused   ///< Simulation paused, state preserved
            };

            /**
             * @brief Construct the engine core.
             * 
             * Creates the core subsystems (logging, timing, settings) and
             * determines the assets path based on the executable location.
             */
            EngineCore();
            
            /**
             * @brief Destroy the engine core.
             * 
             * Ensures proper cleanup of all subsystems.
             */
            ~EngineCore();

            /**
             * @brief Initialize all engine subsystems.
             * 
             * Sets up physics, world, and rendering systems in the correct order,
             * configures render passes, and prepares the engine for operation.
             * Must be called before tick().
             * 
             * @throws std::runtime_error if initialization of any subsystem fails
             */
            void initialise();
            
            /**
             * @brief Update all engine subsystems for one frame.
             * 
             * Advances the timing service, updates the world and physics based on
             * simulation state, and renders the current frame. Should be called
             * once per frame from the main loop.
             */
            void tick();
            
            /**
             * @brief Shutdown all engine subsystems.
             * 
             * Cleanly shuts down rendering, world, and physics systems in the
             * reverse order of initialization. Should be called before destroying
             * the engine core.
             */
            void shutdown();

            /**
             * @brief Get the world system.
             * @return Shared pointer to the world system
             */
            std::shared_ptr<Arche::Scene::WorldSystem> getWorld() const { return worldSystem; }
            
            /**
             * @brief Get the rendering system.
             * @return Shared pointer to the rendering system
             */
            std::shared_ptr<Arche::Render::RenderingSystem> getRenderer() const { return renderingSystem; }
            
            /**
             * @brief Get the logging service.
             * @return Shared pointer to the logging service
             */
            std::shared_ptr<Arche::Core::LoggingService> getLogger() const { return loggingService; }
            
            /**
             * @brief Get the timing service.
             * @return Shared pointer to the timing service
             */
            std::shared_ptr<Arche::Core::TimingService> getTimer() const { return timingService; }
            
            /**
             * @brief Get the physics system.
             * @return Shared pointer to the physics system
             */
            std::shared_ptr<Arche::Physics::PhysicsSystem> getPhysics() const { return physicsSystem; }
            
            /**
             * @brief Get the global settings.
             * @return Reference to the global settings object
             */
            GlobalSettings& getGlobalSettings() { return *globalSettings; }

            /**
             * @brief Toggle simulation between running and paused states.
             * 
             * If running, pauses the simulation. If paused or idle, starts/resumes
             * the simulation.
             */
            void toggleSimulation();
            
            /**
             * @brief Start or resume the simulation.
             * 
             * If idle, starts the simulation and captures initial state snapshot.
             * If paused, resumes from the current state. Has no effect if already running.
             */
            void playSimulation();
            
            /**
             * @brief Pause the simulation.
             * 
             * Freezes the simulation in its current state. Has no effect if not running.
             */
            void pauseSimulation();
            
            /**
             * @brief Reset the simulation to its initial state.
             * 
             * Restores all entities to their positions when the simulation first started,
             * clears the snapshot, and returns to idle state.
             */
            void resetSimulation();
            
            /**
             * @brief Get the current simulation state.
             * @return Current SimulationState value
             */
            SimulationState getSimulationState() const { return simulationState; }
            
            /**
             * @brief Check if simulation is running.
             * @return true if simulation is in Running state, false otherwise
             */
            bool simulationIsRunning() const { return simulationState == SimulationState::Running; }
            
            /**
             * @brief Check if simulation is not running.
             * @return true if simulation is paused or idle, false if running
             */
            bool simulationIsPaused() const { return simulationState != SimulationState::Running; }

          private:
            std::shared_ptr<Arche::Core::LoggingService> loggingService;    ///< Centralized logging service
            std::shared_ptr<Arche::Core::TimingService> timingService;      ///< High-resolution timing service
            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem;         ///< Scene and entity management
            std::shared_ptr<Arche::Render::RenderingSystem> renderingSystem;///< Rendering subsystem
            std::shared_ptr<Arche::Physics::PhysicsSystem> physicsSystem;   ///< Physics simulation
            std::shared_ptr<GlobalSettings> globalSettings;                 ///< Global engine settings
            std::shared_ptr<Render::ShaderLoader> shaderLoader;             ///< Shader resource loader
            std::shared_ptr<Render::MaterialLoader> materialLoader;         ///< Material resource loader

            std::filesystem::path assetsPath;                               ///< Path to asset directory
            SimulationState simulationState{SimulationState::Idle};         ///< Current simulation state
            bool simulationSnapshotCaptured{false};                         ///< Whether initial state was saved
        };

    } // namespace Core
} // namespace Arche