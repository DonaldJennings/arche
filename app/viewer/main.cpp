#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <EngineCore.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <UIContext.h>
#include <WorldConfig.h>
#include <WorldSystem.h>

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
#include <Viewport3D.h>

int main() {
    try {
        // 1. Initialize GLFW and create window
        Arche::GUI::GLFWInitialiser glfwInitialiser;
        auto window = std::make_shared<Arche::GUI::GLFWWindowHandle>(1280, 720, "Arche Engine");

        Arche::Core::WorldConfig config;
        config.stepDuration = 1.0f / 60.0f;
        config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f);
        auto world = Arche::Scene::World::Create(config);

        // 2. Create EngineCore (manages all services)
        auto engineCore = std::make_shared<Arche::Core::EngineCore>(world);

        auto cam{std::make_shared<Arche::Scene::Camera>()};

        cam->SetPosition(Arche::Math::Vector3D(0.0f, 0.0f, 25.0f));
        cam->setPitchYaw(0.0f, 180.0f);
        cam->setPerspective(60.0f,
                            static_cast<double>(engineCore->getRenderer()->getWidth()) /
                                static_cast<double>(engineCore->getRenderer()->getHeight()),
                            0.1f, 1000.0f);

        engineCore->getRenderer()->attachCamera(cam);

        // --- DEBUG: create test particles to validate camera / coordinates ---
        {
            // particle at world origin (should be visible if camera looks at origin)
            Arche::Math::SpatialTransform t0;
            t0.setPosition(Arche::Math::Vector3D(0.0, 0.0, 0.0));
            t0.setScale(Arche::Math::Vector3D(10.0, 10.0, 10.0)); // make large so it's easy to spot
            auto id0 = world->createParticle(t0, 1.0f);
            {
                std::ostringstream oss;
                oss << "DEBUG: created particle id=" << id0 << " at (0,0,0) scale=10";
                ARCHE_LOG_INFO(engineCore->getLoggingService(), oss.str());
            }

            // particle directly in front of camera (5 units ahead)
            auto camPos = cam->GetPosition();
            double pitchDeg = cam->GetPitch();
            double yawDeg = cam->GetYaw();
            auto DegToRad = [](double d) { return d * (3.14159265358979323846 / 180.0); };
            double pr = DegToRad(pitchDeg);
            double yr = DegToRad(yawDeg);
            double fx = std::cos(pr) * std::sin(yr);
            double fy = std::sin(pr);
            double fz = -std::cos(pr) * std::cos(yr);
            Arche::Math::Vector3D forward(static_cast<double>(fx), static_cast<double>(fy), static_cast<double>(fz));
            forward = forward.normalized();
            Arche::Math::SpatialTransform t1;
            auto frontPos = camPos + forward * 5.0; // 5 units in front
            t1.setPosition(frontPos);
            t1.setScale(Arche::Math::Vector3D(8.0, 8.0, 8.0));
            auto id1 = world->createParticle(t1, 1.0f);
            {
                std::ostringstream oss;
                oss << "DEBUG: created particle id=" << id1 << " at (" << frontPos.x() << "," << frontPos.y() << "," << frontPos.z() << ") scale=8";
                ARCHE_LOG_INFO(engineCore->getLoggingService(), oss.str());
            }
        }

        // 4. Create UIContext and pass references to services and WorldSystem
        auto context = std::make_shared<Arche::GUI::UIContext>(engineCore);

        // 5. Setup GUI backend and panel registry
        Arche::GUI::PanelRegistry panels;
        auto guiLogger = std::make_unique<Arche::GUI::GUILogSink>();
        auto *guiLoggerPtr = guiLogger.get();
        context->logger()->addSink(std::move(guiLogger));

        panels.RegisterPanel(std::make_shared<Arche::GUI::DockspacePanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::LogPanel>(guiLoggerPtr));
        panels.RegisterPanel(std::make_shared<Arche::GUI::MetricsPanel>(context));
        panels.RegisterPanel(std::make_shared<Arche::GUI::Viewport3DPanel>(context));

        auto guiRunner = Arche::GUI::GUIRunner(window);
        guiRunner.setDockController([&]() { panels.DrawPanels(); });

        engineCore->initialise(window->get());

        engineCore->getTimingService()->pause();
        // 6. Main loop
        while (!glfwWindowShouldClose(*window)) {
            engineCore->update();

            glfwPollEvents();
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