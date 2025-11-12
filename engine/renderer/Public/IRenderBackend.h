#pragma once
#include "World.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <array>
#include <memory>

#include <Camera.h>
#include <IEntity.h>

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

            void render(std::vector<std::shared_ptr<Arche::Scene::IEntity>> const &entities);
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

            void setClearColour(float r, float g, float b, float a) { clearColor = {r, g, b, a}; }

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

            glm::vec4 clearColor{0.1f, 0.12f, 0.15f, 1.0f};
        };

    } // namespace Render
} // namespace Arche