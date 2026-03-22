#pragma once
#include "IRenderPass.h"
#include "BvhBuilder.h"      // GpuSphere, GpuMaterial, GpuBvhNode, BvhBuilder
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <cstdint>
#include <cstring>

#ifdef ARCHE_BACKEND_VULKAN
#   include "VulkanBackend.h"
#endif

namespace Arche {
    namespace Render {

        /**
         * @brief Progressive GPU path-tracing render pass (Vulkan only).
         *
         * When path tracing is enabled (settings.globalSettings.pathTrace.enabled),
         * this pass:
         *  1. Extracts spheres from the RenderScene by inspecting transform scale.
         *  2. Maps material names to GpuMaterial using the ResourceRegistry.
         *  3. Builds a BVH via BvhBuilder.
         *  4. Calls VulkanBackend::dispatchPathTrace() which accumulates samples
         *     and tone-maps into the offscreen colour target.
         *
         * On each call the scene snapshot is compared to the previous call; if
         * anything changed the accumulation buffer is reset to 0.
         */
        class PathTracingPass : public IRenderPass {
          public:
#ifdef ARCHE_BACKEND_VULKAN
            explicit PathTracingPass(VulkanBackend *vulkanBackend)
                : m_vulkan(vulkanBackend) {}
#endif
            PathTracingPass() = default;

            void initialise(IRenderBackend &) override {}

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
                std::size_t              sceneHash = 1469598103934665603ull; // FNV-1a seed

                auto hashCombineU32 = [&](uint32_t v) {
                    sceneHash ^= static_cast<std::size_t>(v);
                    sceneHash *= 1099511628211ull;
                };
                auto hashCombineF32 = [&](float v) {
                    static_assert(sizeof(float) == sizeof(uint32_t));
                    uint32_t bits = 0;
                    std::memcpy(&bits, &v, sizeof(bits));
                    hashCombineU32(bits);
                };

                for (const auto &obj : view.opaqueObjects) {
                    if (obj.meshId != "uv_sphere.mesh") continue;

                    // Extract world centre and radius from the model matrix
                    glm::vec3 centre = glm::vec3(obj.transform[3]);
                    float     radius = glm::length(glm::vec3(obj.transform[0])); // scale X column

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

                    // Track scene content changes (transform/material edits, etc.)
                    hashCombineF32(centre.x);
                    hashCombineF32(centre.y);
                    hashCombineF32(centre.z);
                    hashCombineF32(radius);
                    hashCombineF32(mat.albedo.x);
                    hashCombineF32(mat.albedo.y);
                    hashCombineF32(mat.albedo.z);
                    hashCombineF32(mat.fuzzOrIOR);
                    hashCombineU32(static_cast<uint32_t>(mat.type));
                }

                if (spheres.empty()) return;

                // ── Build BVH ─────────────────────────────────────────────────
                std::vector<GpuBvhNode> bvhNodes = BvhBuilder::build(spheres);

                // ── Detect scene or settings change → reset accumulation ──────
                bool reset = false;
                if (m_prevSphereCount    != spheres.size())            reset = true;
                if (m_prevMaxBounces     != pts.maxBounces)            reset = true;
                if (m_prevSamplesPerFrame!= pts.samplesPerFrame)       reset = true;
                if (m_prevAperture       != pts.aperture)              reset = true;
                if (m_prevFocusDist      != pts.focusDistance)         reset = true;
                if (m_prevSceneHash      != sceneHash)                 reset = true;

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
                    m_prevSceneHash       = sceneHash;
                }

                // ── Build camera push constants ───────────────────────────────
                PathTracePushConstants pc{};

                // Extract camera axes from view matrix (column-major GLM)
                glm::vec3 right  = { vmat[0][0], vmat[1][0], vmat[2][0] };
                glm::vec3 up     = { vmat[0][1], vmat[1][1], vmat[2][1] };
                glm::vec3 fwd    = {-vmat[0][2],-vmat[1][2],-vmat[2][2] }; // camera looks in -Z

                float fovRad     = glm::radians(settings.globalSettings.fieldOfView);
                float halfH      = std::tan(fovRad * 0.5f);

                // Viewport dimensions from backend
                // Use offscreen extent from a fixed aspect ratio approximation
                float aspect = static_cast<float>(view.projectionMatrix[0][0]);  // proj[0][0] = 1/tan(fov/2)*1/aspect
                // Better: derive aspect from projection matrix: proj[0][0] = cot(fov/2)/aspect
                // aspect = (1/tan(fov/2)) / proj[0][0]
                float cotFov = 1.0f / std::tan(fovRad * 0.5f);
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
                // imageW/imageH set inside dispatchPathTrace from offscreenExtent

                m_accumSamples += static_cast<uint32_t>(pts.samplesPerFrame);
                ++m_frameIndex;

                m_vulkan->dispatchPathTrace(spheres, materials, bvhNodes,
                                            pc, m_accumSamples, reset);
#endif
            }

            void shutdown(IRenderBackend &) override {}

            /** @brief Reset the progressive accumulation (e.g. after scene edit). */
            void resetAccumulation() {
                m_accumSamples = 0;
                m_frameIndex   = 0;
                m_prevViewMatrix = glm::mat4(0.0f); // force reset next frame
            }

            uint32_t getTotalSamples() const { return m_accumSamples; }

          private:
#ifdef ARCHE_BACKEND_VULKAN
            VulkanBackend *m_vulkan{nullptr};
#endif
            uint32_t  m_frameIndex{0};
            uint32_t  m_accumSamples{0};
            glm::mat4 m_prevViewMatrix{0.0f};

            // Previous-frame state for change detection
            uint32_t  m_prevSphereCount{0};
            int       m_prevMaxBounces{0};
            int       m_prevSamplesPerFrame{0};
            float     m_prevAperture{-1.0f};
            float     m_prevFocusDist{-1.0f};
            std::size_t m_prevSceneHash{0};
        };

    } // namespace Render
} // namespace Arche
