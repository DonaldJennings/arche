#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <EngineCore.h>
#include <LoggingService.h>
#include <JobPoolService.h>
#include <TimingService.h>
#include <WorldSystem.h>
#include <WorldConfig.h>
#include <UIContext.h>

#include <DockspacePanel.h>
#include <GUILogSink.h>
#include <ImGuiBackend.h>
#include <LogPanel.h>
#include <MetricsPanel.h>
#include <PanelRegistry.h>
#include <Viewport2DPanel.h>

#include <GLFWInitialiser.h>
#include <GLFWWindow_RAII.h>
#include <GUIRunner.h>

int main() {
    try {
        // 1. Initialize GLFW and create window
        Arche::GUI::GLFWInitialiser glfwInitialiser;
        auto window = std::make_shared<Arche::GUI::GLFWWindowHandle>(1280, 720, "Arche Engine");

        // 2. Create EngineCore (manages all services)
        auto engineCore = std::make_shared<Arche::Core::EngineCore>();

        // 3. Create World and WorldSystem subsystem
        Arche::Core::WorldConfig config;
        config.stepDuration = 1.0f / 60.0f;
        config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f);

        auto world = Arche::Scene::World::Create(config);
        auto worldSystem = std::make_shared<Arche::Scene::WorldSystem>(
            world,
            engineCore->getLoggingService(),
            engineCore->getJobPoolService(),
            engineCore->getTimingService()
        );
        engineCore->registerSubsystem(worldSystem);

        // 4. Create UIContext and pass references to services and WorldSystem
        auto context = std::make_shared<Arche::GUI::UIContext>(
            engineCore->getLoggingService(),
            worldSystem
        );

        // 5. Setup GUI backend and panel registry
        Arche::GUI::PanelRegistry panels;
        auto guiLogger = std::make_unique<Arche::GUI::GUILogSink>();
        auto* guiLoggerPtr = guiLogger.get();
        context->logger->addSink(std::move(guiLogger));

        panels.RegisterPanel(std::make_shared<Arche::GUI::DockspacePanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::LogPanel>(guiLoggerPtr));
        panels.RegisterPanel(std::make_shared<Arche::GUI::MetricsPanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::Viewport2DPanel>(context));

        auto guiRunner = Arche::GUI::GUIRunner(window);
        guiRunner.setDockController([&]() {
            panels.DrawPanels();
        });

        engineCore->initialise();

        // 6. Main loop
        while (!glfwWindowShouldClose(*window)) {
            glfwPollEvents();
            guiRunner.frame();
            glfwSwapBuffers(*window);
        }

        engineCore->shutdown();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}