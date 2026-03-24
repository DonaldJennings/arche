#pragma once
#include "IRenderPass.h"
#include "BvhBuilder.h"      // GpuSphere, GpuMaterial, GpuBvhNode, BvhBuilder
#include "Camera.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

#ifdef ARCHE_BACKEND_VULKAN
#   include "VulkanBackend.h"
#endif

namespace Arche {
    namespace Render {

        /**
         * @brief Progressive GPU path-tracing render pass (Vulkan only).
         *
         * Implements the CPU-side orchestration of the two-pass path-tracing pipeline:
         *
         * **Pass 1 — path_trace.comp**
         *   Each invocation fires @c samplesPerFrame camera rays and accumulates
         *   radiance into a persistent RGBA32F storage image (@c m_ptAccumImage).
         *   R/G/B channels hold the sum of sample radiance; the sample count is
         *   tracked externally via @c m_accumSamples.
         *
         * **Pass 2 — accumulate.comp**
         *   Divides the accumulated radiance by @c totalSamples, applies gamma-2
         *   correction (sqrt), and writes the result to the 8-bit offscreen colour
         *   target that ImGui samples.
         *
         * **Progressive accumulation** — as long as the scene, camera, and
         * rendering settings remain unchanged, successive dispatches add more
         * samples to the same accumulation buffer, progressively converging to a
         * noise-free image.  Any change (sphere count, camera matrix, aperture,
         * focus, bounce/sample counts) resets the buffer to zero.
         *
         * **Scene representation** — the path tracer is sphere-only; every opaque
         * object in the @c RenderView is approximated as a bounding sphere derived
         * from its world-space transform scale.
         *
         * @note Only active when @c ARCHE_BACKEND_VULKAN is defined and a
         *       VulkanBackend pointer is provided at construction time.  The
         *       OpenGL code path returns immediately from render().
         */
        class PathTracingPass : public IRenderPass {
          public:
#ifdef ARCHE_BACKEND_VULKAN
            /**
             * @brief Construct with the Vulkan backend that owns the GPU resources.
             *
             * @param vulkanBackend  Non-owning pointer to the active VulkanBackend.
             *                       Must remain valid for the lifetime of this pass.
             */
            explicit PathTracingPass(VulkanBackend *vulkanBackend)
                : m_vulkan(vulkanBackend) {}
#endif
            /** @brief Default constructor (non-Vulkan builds / null backend). */
            PathTracingPass() = default;

            /**
             * @brief No-op initialisation — GPU resources are lazily allocated on the
             *        first @c render() call via VulkanBackend::dispatchPathTrace().
             *
             * @param backend  Unused.
             */
            void initialise(IRenderBackend &) override {}

            /**
             * @brief Execute one path-trace frame.
             *
             * Performs the following steps each frame:
             *  1. Build per-sphere @c GpuSphere and @c GpuMaterial lists from the
             *     opaque objects in @p view.  Each object is approximated as a
             *     bounding sphere whose radius is the maximum world-space scale axis.
             *  2. Build a flat BVH over the sphere list using @c BvhBuilder::build().
             *  3. Detect scene / settings changes and set a reset flag if any differ
             *     from the previous frame.
             *  4. Compute camera parameters (lower-left corner, horizontal / vertical
             *     spans, lens disc) from the view matrix, FOV, aperture, and focus
             *     distance.
             *  5. Fill a @c PathTracePushConstants struct.  Note: @c imageW and
             *     @c imageH are intentionally left at zero here and are injected by
             *     @c VulkanBackend::dispatchPathTrace() from @c m_offscreenExtent.
             *  6. Call @c VulkanBackend::dispatchPathTrace() with the accumulated
             *     sample count and the reset flag.
             *
             * @param view      View data: opaque object list, view/projection matrices,
             *                  camera world position.
             * @param backend   Render backend (cast internally to VulkanBackend on
             *                  Vulkan builds; unused on OpenGL builds).
             * @param camera    Active scene camera (used indirectly via @p view matrices).
             * @param resources Resource registry for material property lookup.
             * @param settings  Global render settings including path-trace parameters.
             */
            void render(const RenderView      &view,
                        IRenderBackend        &backend,
                        const Camera          &camera,
                        ResourceRegistry      &resources,
                        RenderPassSettings    &settings) override
            {
#ifndef ARCHE_BACKEND_VULKAN
                return;
#else
                if (!m_vulkan) return;
                if (!settings.globalSettings.pathTrace.enabled) return;

                const auto &pts = settings.globalSettings.pathTrace;

                // ── Build sphere + material lists ─────────────────────────────
                std::vector<GpuSphere>   spheres;
                std::vector<GpuMaterial> materials;

                for (const auto &obj : view.opaqueObjects) {
                    // Current path tracer primitive set is sphere-only.
                    // Approximate every opaque object as a bounding sphere so
                    // editor scenes remain visible in path-trace mode.
                    glm::vec3 centre = glm::vec3(obj.transform[3]);
                    float sx = glm::length(glm::vec3(obj.transform[0]));
                    float sy = glm::length(glm::vec3(obj.transform[1]));
                    float sz = glm::length(glm::vec3(obj.transform[2]));
                    float radius = std::max(sx, std::max(sy, sz));

                    // Build GpuMaterial from the material resource
                    GpuMaterial mat{};
                    auto matRes = resources.getMaterial(obj.materialId);
                    if (matRes) {
                        // Albedo: prefer uAlbedo vec3, fall back to baseColor
                        auto albedoV = matRes->getVec3("uAlbedo");
                        if (albedoV) {
                            mat.albedo = *albedoV;
                        } else {
                            mat.albedo = glm::vec3(matRes->getBaseColor());
                        }
                        // Material type: 0=Lambertian 1=Metal 2=Dielectric
                        auto typeF = matRes->getFloat("uMaterialType");
                        mat.type = typeF ? static_cast<int>(*typeF) : 0;

                        auto fuzz = matRes->getFloat("uFuzz");
                        auto ior  = matRes->getFloat("uIOR");
                        if (mat.type == 1 && fuzz) mat.fuzzOrIOR = *fuzz;
                        if (mat.type == 2 && ior)  mat.fuzzOrIOR = *ior;
                        if (mat.type == 2 && !ior)  mat.fuzzOrIOR = 1.5f; // glass default
                    }

                    GpuSphere s{};
                    s.center = centre;
                    s.radius = radius;
                    s.matIdx = static_cast<int>(materials.size());
                    spheres.push_back(s);
                    materials.push_back(mat);
                }

                if (spheres.empty()) return;

                // ── Build BVH ─────────────────────────────────────────────────
                std::vector<GpuBvhNode> bvhNodes = BvhBuilder::build(spheres);

                // ── Detect scene or settings change → reset accumulation ──────
                bool reset = false;
                if (m_prevSphereCount != spheres.size())
                {
                    reset = true;
                }
                else
                {
                    // Check for changes in individual spheres (position, radius, material)
                    for (size_t i = 0; i < spheres.size(); ++i) {
                        if (spheres[i].center != m_prevSpheres[i].center || spheres[i].radius != m_prevSpheres[i].radius)
                        {
                            reset = true;
                            break;
                        }
                    }
                }
                if (m_prevMaxBounces     != pts.maxBounces)            reset = true;
                if (m_prevSamplesPerFrame!= pts.samplesPerFrame)       reset = true;
                if (m_prevAperture       != pts.aperture)              reset = true;
                if (m_prevFocusDist      != pts.focusDistance)         reset = true;

                m_prevSpheres = spheres; // store current sphere state for next frame's change detection

                // Camera change detection
                glm::mat4 vmat = view.viewMatrix;
                if (vmat != m_prevViewMatrix) { reset = true; m_prevViewMatrix = vmat; }

                if (reset) {
                    m_accumSamples  = 0;
                    m_frameIndex    = 0;
                    m_prevSphereCount     = static_cast<uint32_t>(spheres.size());
                    m_prevMaxBounces      = pts.maxBounces;
                    m_prevSamplesPerFrame = pts.samplesPerFrame;
                    m_prevAperture        = pts.aperture;
                    m_prevFocusDist       = pts.focusDistance;
                }

                // ── Build camera push constants ───────────────────────────────
                PathTracePushConstants pc{};

                // Extract camera axes from view matrix (column-major GLM)
                glm::vec3 right  = { vmat[0][0], vmat[1][0], vmat[2][0] };
                glm::vec3 up     = { vmat[0][1], vmat[1][1], vmat[2][1] };
                glm::vec3 fwd    = {-vmat[0][2],-vmat[1][2],-vmat[2][2] }; // camera looks in -Z

                float fovRad     = glm::radians(settings.globalSettings.fieldOfView);
                float halfH      = std::tan(fovRad * 0.5f);

                // Derive aspect ratio from the projection matrix:
                //   proj[0][0] = cot(fov/2) / aspect  =>  aspect = cot(fov/2) / proj[0][0]
                float cotFov     = 1.0f / std::tan(fovRad * 0.5f);
                float aspectRatio = cotFov / view.projectionMatrix[0][0];

                float halfW      = halfH * aspectRatio;
                float focusDist  = pts.focusDistance;

                pc.cameraOrigin = view.cameraPosition;
                pc.lensRadius   = pts.aperture * 0.5f;
                pc.lensU        = right;
                pc.lensV        = up;

                pc.lowerLeft   = view.cameraPosition
                                 - halfW  * focusDist * right
                                 - halfH  * focusDist * up
                                 + focusDist * fwd;
                pc.horizontal  = 2.0f * halfW  * focusDist * right;
                pc.vertical    = 2.0f * halfH  * focusDist * up;

                pc.frameIndex      = m_frameIndex;
                pc.samplesPerFrame = static_cast<uint32_t>(pts.samplesPerFrame);
                pc.maxBounces      = static_cast<uint32_t>(pts.maxBounces);
                pc.sphereCount     = static_cast<uint32_t>(spheres.size());
                pc.bvhNodeCount    = static_cast<uint32_t>(bvhNodes.size());
                // pc.imageW / pc.imageH are injected by dispatchPathTrace() from
                // VulkanBackend::m_offscreenExtent, which is not visible here.

                m_accumSamples += static_cast<uint32_t>(pts.samplesPerFrame);
                ++m_frameIndex;

                m_vulkan->dispatchPathTrace(spheres, materials, bvhNodes,
                                            pc, m_accumSamples, reset);
#endif
            }

            /**
             * @brief No-op shutdown — GPU resources are owned by VulkanBackend.
             *
             * @param backend  Unused.
             */
            void shutdown(IRenderBackend &) override {}

            /**
             * @brief Force-reset the progressive accumulation on the next frame.
             *
             * Zeroes the accumulated sample counter and frame index, and marks the
             * previous view matrix as invalid so the next dispatch always passes
             * @c resetAccum = true to VulkanBackend::dispatchPathTrace(), clearing
             * the RGBA32F accumulation image before adding new samples.
             *
             * Call this whenever a setting that affects rendering changes externally
             * (e.g. from the RayTracingPanel).
             */
            void resetAccumulation() {
                m_accumSamples = 0;
                m_frameIndex   = 0;
                m_prevViewMatrix = glm::mat4(0.0f); // force reset next frame
            }

            /**
             * @brief Return the total number of samples accumulated since the last reset.
             *
             * Displayed in the Ray Tracing panel as a convergence indicator.
             *
             * @return Number of path-trace samples accumulated in the current image.
             */
            uint32_t getTotalSamples() const { return m_accumSamples; }

          private:
#ifdef ARCHE_BACKEND_VULKAN
            VulkanBackend *m_vulkan{nullptr};  ///< Non-owning pointer to the Vulkan backend.
#endif
            uint32_t  m_frameIndex{0};      ///< Monotonically increasing per-frame seed for the RNG.
            uint32_t  m_accumSamples{0};    ///< Total samples accumulated since last reset.
            glm::mat4 m_prevViewMatrix{0.0f}; ///< Previous frame's view matrix for change detection.

            // ── Previous-frame state (change detection) ──────────────────────
            uint32_t  m_prevSphereCount{0};    ///< Sphere count from the previous frame.
            int       m_prevMaxBounces{0};     ///< Max bounce depth from the previous frame.
            int       m_prevSamplesPerFrame{0};///< Samples-per-frame count from the previous frame.
            float     m_prevAperture{-1.0f};   ///< Aperture value from the previous frame.
            float     m_prevFocusDist{-1.0f};  ///< Focus distance from the previous frame.

            std::vector<GpuSphere> m_prevSpheres; ///< Previous frame's sphere list for change detection.
        };

    } // namespace Render
} // namespace Arche
