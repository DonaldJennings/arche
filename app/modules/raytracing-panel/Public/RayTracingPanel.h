#pragma once
#include <IPanel.h>
#include <EditorSession.h>
#include <memory>

#ifdef ARCHE_BACKEND_VULKAN
#   include <PathTracingPass.h>
#endif

namespace Arche {
    namespace GUI {

        /**
         * @brief ImGui panel for configuring the GPU path tracer.
         *
         * Exposes all PathTraceSettings fields with appropriate widgets,
         * a sample accumulation counter, and a Reset button.
         */
        class RayTracingPanel : public IPanel {
          public:
            explicit RayTracingPanel(std::shared_ptr<EditorSession> ctx
#ifdef ARCHE_BACKEND_VULKAN
                , Render::PathTracingPass *ptPass = nullptr
#endif
            )
                : m_ctx(std::move(ctx))
#ifdef ARCHE_BACKEND_VULKAN
                , m_ptPass(ptPass)
#endif
            {}

            void Draw() override;
            std::string_view GetName() const override { return "Ray Tracing"; }

          private:
            std::shared_ptr<EditorSession> m_ctx;
#ifdef ARCHE_BACKEND_VULKAN
            Render::PathTracingPass *m_ptPass{nullptr};
#endif
        };

    } // namespace GUI
} // namespace Arche
