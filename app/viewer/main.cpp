#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <EngineCore.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <UIContext.h>
#include <WorldSystem.h>
#include <Camera.h> // Include the Camera header


#include <DockspacePanel.h>
#include <GUILogSink.h>
#include <ImGuiBackend.h>
#include <LogPanel.h>
#include <MetricsPanel.h>
#include <MaterialBrowser.h>
#include <ShaderBrowser.h>
#include <PanelRegistry.h>
#include <WorldProperties.h>

#include <SphereEntity.h>

#include <GLFWInitialiser.h>
#include <GLFWWindow_RAII.h>
#include <GUIRunner.h>
#include <Viewport3D.h>

int main() {
    try {
        // 1. Initialize GLFW and create window
        Arche::GUI::GLFWInitialiser glfwInitialiser;
        auto window = std::make_shared<Arche::GUI::GLFWWindowHandle>(1920, 1080, "Arche Engine");

        // 2. Create EngineCore (manages all services)
        auto engineCore = std::make_shared<Arche::Core::EngineCore>();

        // 3. Initialize engine systems
        engineCore->initialise();

        // Create and set the main camera
        auto mainCamera = std::make_shared<Arche::Render::Camera>();
        mainCamera->setPerspective(60.0, 1920.0 / 1080.0, 0.1, 1000.0);
        mainCamera->SetPosition({0.0, 5.0, 20.0});
        engineCore->getRenderer()->setMainCamera(mainCamera);

        // 4. Create UIContext and pass references to services and WorldSystem
        auto context = std::make_shared<Arche::GUI::UIContext>(engineCore);

        // 5. Setup GUI backend and panel registry
        Arche::GUI::PanelRegistry panels;
        auto guiLogger = std::make_unique<Arche::GUI::GUILogSink>();
        auto *guiLoggerPtr = guiLogger.get();
        engineCore->getLogger()->addSink(std::move(guiLogger));

        panels.RegisterPanel(std::make_shared<Arche::GUI::DockspacePanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::LogPanel>(guiLoggerPtr));
        panels.RegisterPanel(std::make_shared<Arche::GUI::MetricsPanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::Viewport3DPanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::WorldProperties>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::MaterialBrowser>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::ShaderBrowser>(context));

        auto guiRunner = Arche::GUI::GUIRunner(window);
        guiRunner.setDockController([&]() { panels.DrawPanels(); });

        engineCore->getWorld()->addEntity(
            std::make_shared<Arche::Scene::SphereEntity>(2.50f, glm::vec3(0.0f, 10.0f, -15.0f)));

        engineCore->getTimer()->pause();
        
        // 6. Main loop
        while (!glfwWindowShouldClose(*window)) {
            glfwPollEvents();

            // Tick the engine systems
            engineCore->tick();

            // Render the GUI
            guiRunner.frame();

            glfwSwapBuffers(*window);
        }

        engineCore->shutdown();
    } catch (const std::exception &ex) {

        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}