#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#ifndef ARCHE_BACKEND_VULKAN
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <EngineCore.h>
#include <LoggingService.h>
#include <EditorSession.h>
#include <WorldSystem.h>
#include <Camera.h>

#include <DockspacePanel.h>
#include <GUILogSink.h>
#include <ImGuiBackend.h>
#include <LogPanel.h>
#include <MetricsPanel.h>
#include <MaterialBrowser.h>
#include <ShaderBrowser.h>
#include <EntityInspector.h>
#include <PanelRegistry.h>
#include <WorldProperties.h>

#include <SphereEntity.h>

#include <GLFWInitialiser.h>
#include <GLFWWindow_RAII.h>
#include <GUIRunner.h>
#include <Viewport3D.h>

#ifdef ARCHE_BACKEND_VULKAN
#   include <VulkanBackend.h>
#   include <RenderingSystem.h>
#endif
#include <RayTracingPanel.h>

#ifdef ARCHE_BACKEND_VULKAN
#   include <PathTracingPass.h>
#endif

int main() {
    try {
        // 1. Initialize GLFW and create window
        Arche::GUI::GLFWInitialiser glfwInitialiser;
        auto window = std::make_shared<Arche::GUI::GLFWWindowHandle>(1920, 1080, "Arche Engine");

        // 2. Create EngineCore (manages all services)
        auto engineCore = std::make_shared<Arche::Core::EngineCore>();

        // 3. Initialize engine systems (window passed for Vulkan surface creation)
        engineCore->initialise(window->get());

        // Create and set the main camera
        auto mainCamera = std::make_shared<Arche::Render::Camera>();
        mainCamera->setPerspective(60.0, 1920.0 / 1080.0, 0.1, 1000.0);
        mainCamera->SetPosition({13, 2, 3});
        mainCamera->setPitchYaw(-8.5, 192.9);
        engineCore->getRenderer()->setMainCamera(mainCamera);

        auto centerSphere = std::make_shared<Arche::Scene::SphereEntity>(1.0f, glm::vec3(0, 1, 0), false);
        centerSphere->setMaterialId("sphere_lambertian.mat");

        auto leftSphere = std::make_shared<Arche::Scene::SphereEntity>(1.0f, glm::vec3(-4, 1, 0), false);
        leftSphere->setMaterialId("sphere_glass.mat");

        auto rightSphere = std::make_shared<Arche::Scene::SphereEntity>(1.0f, glm::vec3(4, 1, 0), false);
        rightSphere->setMaterialId("sphere_metal.mat");

        engineCore->getWorld()->addEntity(centerSphere);
        engineCore->getWorld()->addEntity(leftSphere);
        engineCore->getWorld()->addEntity(rightSphere);

      
        // 4. Create the editor session wrapper around core engine services
        auto context = std::make_shared<Arche::GUI::EditorSession>(engineCore);

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
        panels.RegisterPanel(std::make_shared<Arche::GUI::EntityInspector>(context));

#ifdef ARCHE_BACKEND_VULKAN
        // Retrieve the VulkanBackend from the RenderingSystem via the new getBackend() accessor.
        // The static_cast is safe: under ARCHE_BACKEND_VULKAN the backend is always VulkanBackend.
        Arche::Render::VulkanBackend *vulkanBackend =
            static_cast<Arche::Render::VulkanBackend *>(
                engineCore->getRenderer()->getBackend());

        auto guiRunner = Arche::GUI::GUIRunner(window, vulkanBackend);

        
#ifdef ARCHE_BACKEND_VULKAN
        auto *ptPass = new Arche::Render::PathTracingPass(vulkanBackend);
        engineCore->getRenderer()->addRenderPass(std::shared_ptr<Arche::Render::IRenderPass>(ptPass));
#endif

        
#ifdef ARCHE_BACKEND_VULKAN
        panels.RegisterPanel(std::make_shared<Arche::GUI::RayTracingPanel>(context, ptPass));
#else
        panels.RegisterPanel(std::make_shared<Arche::GUI::RayTracingPanel>(context));
#endif

#else
        auto guiRunner = Arche::GUI::GUIRunner(window);
#endif
        guiRunner.setDockController([&]() { panels.DrawPanels(); });

        engineCore->getWorld()->addEntity(
            std::make_shared<Arche::Scene::SphereEntity>(2.50f, glm::vec3(0.0f, 10.0f, -15.0f)));

        engineCore->resetSimulation();

        // 6. Main loop
        while (!glfwWindowShouldClose(*window)) {
            glfwPollEvents();

            // Tick the engine systems (physics + geometry pass, no present yet)
            engineCore->tick();

            // Build and render the GUI (ImGui::Render() makes draw data available)
            guiRunner.frame();

            // Present: Vulkan runs the ImGui pass then submits + presents.
            // OpenGL: no-op here; swap buffers below.
            engineCore->getRenderer()->present();

#ifndef ARCHE_BACKEND_VULKAN
            glfwSwapBuffers(*window);
#endif
        }

        // GUI must be shut down before the Vulkan device is destroyed.
        guiRunner.shutdown();
        engineCore->shutdown();
    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}
