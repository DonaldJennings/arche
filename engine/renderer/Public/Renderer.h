#pragma once
#include "World.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <array>
#include <memory>

#include <Camera.h>
struct GLFWwindow;

namespace Arche {
    namespace Render {

        class Renderer {
          public:
            Renderer(std::shared_ptr<Arche::Core::LoggingService> logger, int width = 800, int height = 600,
                     const std::string &title = "Arche Renderer")
                : mLogger{logger}, width{width}, height{height}, title{title} {}

            ~Renderer() noexcept { shutdown(); }

            // Initialise with an optional existing GLFWwindow (the application's main window).
            // If 'mainWindow' is provided, the renderer will create its FBO/texture in that context so the UI can
            // sample it.
            bool initialise(GLFWwindow *mainWindow = nullptr);

            void render(std::vector<Arche::Scene::BodyView> const &entities);
            void shutdown() noexcept;

            unsigned int getRenderTexture() const { return renderTexture; }

            int getWidth() const { return width; }
            int getHeight() const { return height; }

            void setViewportSize(int newWidth, int newHeight) {
                width = newWidth;
                height = newHeight;
            }

            void attachCamera(std::shared_ptr<Arche::Scene::Camera> camera) { mainViewportCamera = camera; }
            // Recreate framebuffer attachments for the current width/height.
            bool recreateFrameBuffer();

            std::shared_ptr<Arche::Scene::Camera> getAttachedCamera() const { return mainViewportCamera; }

          private:
            int width, height;
            std::string title;

            // If window is non-null we created a hidden window; otherwise we used the external mainWindow context.
            GLFWwindow *window = nullptr;
            GLFWwindow *externalContextWindow = nullptr; // if initialise(mainWindow) passed
            unsigned int frameBuffer = 0;
            unsigned int renderTexture = 0;
            unsigned int renderBuffer = 0;

            bool initialised = false;

            bool initialiseFrameBuffer();

            std::shared_ptr<Arche::Core::LoggingService> mLogger;
            std::shared_ptr<Scene::Camera> mainViewportCamera;
        };

    } // namespace Render
} // namespace Arche