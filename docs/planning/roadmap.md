# Arche Engine — Feature Roadmap

## Milestones

| Phase | Title | Description |
|-------|-------|-------------|
| 1 | Architecture Foundation | Physics decoupling, SoA data layout, render data decoupling. Prerequisite for all GPU work. |
| 2 | CPU Parallelism | OpenMP parallel physics, fixed timestep loop. |
| 3 | Vulkan Foundation | Vulkan backend, geometry rendering, ImGui integration, resource management. |
| 4 | GPU Compute | Physics integration on GPU via Vulkan compute shaders, persistent GPU buffers. |
| 5 | Ray Tracing Foundation | Scene geometry buffers, BVH construction, primary rays, direct lighting. |
| 6 | Path Tracer | Full path tracing kernel, GGX materials, temporal accumulation, denoising. |

---

## Phase 1 — Architecture Foundation

### F-01: Physics decoupling refactor
**Labels:** `phase:architecture` `type:refactor`

Remove the raw `glm::vec3*` pointer from `PhysicsBody`. `PhysicsSystem` takes ownership of position state per body. `WorldSystem` syncs positions back to entities after each physics update via entity ID lookup.

**Key changes**
- Add `entityId` to `PhysicsBody`, change `position` from `glm::vec3*` to `glm::vec3` value
- Update `PhysicsSystem::update()` to remove pointer dereference
- Update `WorldSystem::insertEntity` to copy position by value
- Add position sync loop in `WorldSystem::update()` after physics step
- Remove `getPositionPtr()` from `IEntity` and all implementations
- Fix colliders (`SphereCollider`, `CubeCollider`) to remove stored `glm::vec3*`
- Fix `removeEntity` bug: also remove from `PhysicsSystem` via entity ID

**Files:** `engine/physics/Public/PhysicsSystem.h`, `engine/scene/Private/WorldSystem.cpp`, `engine/scene/Public/WorldSystem.h`, `engine/scene/Public/IEntity.h`, `SphereEntity.h`, `CubeEntity.h`, `PlaneEntity.h`, `SphereCollider.h`, `CubeCollider.h`

**Dependencies:** None — this is the first step.

---

### F-02: SoA physics data layout
**Labels:** `phase:architecture` `type:refactor`

Replace `std::vector<PhysicsBody>` (AoS) with parallel arrays (SoA) inside `PhysicsSystem`. This is the data format that maps directly to GPU storage buffers.

**Key changes**
- Replace `m_bodies` with parallel arrays: `positions[]`, `velocities[]`, `accelerations[]`, `masses[]`, `isStatic[]`, `useGravity[]`, `entityIds[]`
- Update `update()` to iterate the flat arrays
- Update `addBody()` and `removeBody()` to maintain parallel arrays
- Update `WorldSystem` position sync to read from `positions[]` by index

**Files:** `engine/physics/Public/PhysicsSystem.h`

**Dependencies:** F-01 must be merged first.

**Why:** GPU buffers require flat contiguous arrays. SoA also improves CPU cache utilisation and is a prerequisite for OpenMP SIMD vectorisation.

---

### F-03: Render data decoupling
**Labels:** `phase:architecture` `type:refactor`

Introduce a `RenderScene` struct that `WorldSystem` populates each frame and passes to `RenderingSystem::render()`. Removes the direct `#include "WorldSystem.h"` dependency from `RenderingSystem`.

**Key changes**
- Add `RenderScene` struct: list of `RenderObject` (mesh ID, material ID, transform matrix)
- `WorldSystem` builds a `RenderScene` each frame from the entity map
- `RenderingSystem::render()` accepts `const RenderScene&` instead of `const WorldSystem&`
- Remove `#include "WorldSystem.h"` from `RenderingSystem.h`

**Files:** `engine/renderer/Public/RenderingSystem.h`, `engine/scene/Public/WorldSystem.h`, new `engine/renderer/Public/RenderScene.h`

**Dependencies:** Can be done in parallel with F-01/F-02, but must be merged before Vulkan work starts.

---

## Phase 2 — CPU Parallelism

### F-04: OpenMP parallel physics integration
**Labels:** `phase:hpc-cpu` `type:performance`

Parallelise the physics integration loop using OpenMP. After F-02 (SoA layout) each loop iteration is fully independent with no shared mutable state.

**Key changes**
- Add `find_package(OpenMP)` to `engine/physics/CMakeLists.txt`
- Add `#pragma omp parallel for schedule(static)` to the integration loop in `PhysicsSystem::update()`
- Verify no data races (each thread writes to a different index in the flat arrays)
- Run existing physics tests to confirm correctness

**Files:** `engine/physics/Public/PhysicsSystem.h`, `engine/physics/CMakeLists.txt`

**Dependencies:** F-02 (SoA layout) must be merged first — parallel iteration over AoS with pointer members is unsafe.

---

### F-05: Fixed timestep physics loop
**Labels:** `phase:hpc-cpu` `type:feature`

Decouple the physics update rate from the frame rate using an accumulator pattern. Physics steps at a fixed interval (e.g. 120 Hz); rendering interpolates between states.

**Key changes**
- Add accumulator in `EngineCore::tick()`
- Physics `update()` called N times per frame with a fixed `dt` until the accumulator is drained
- Store previous and current position per body for render interpolation
- Expose fixed timestep as a setting in `GlobalSettings`

**Files:** `engine/core/Public/EngineCore.h`, `engine/core/Private/EngineCore.cpp`, `engine/core/Public/GlobalSettings.h`

**Dependencies:** F-01. Independent of F-02 and F-04.

**Why:** Required for stable, deterministic simulation before GPU physics is introduced. Frame-rate-dependent physics produces inconsistent results at high or low framerates.

**Deferred:** Render interpolation (blending between previous/current physics position using `accumulator / fixedDt` as alpha) is not yet implemented. Observable only at low fixed rates — revisit before Vulkan work.

---

## Phase 3 — Vulkan Foundation

### F-06: Vulkan device initialisation
**Labels:** `phase:vulkan` `type:feature`

Create a `VulkanBackend` class implementing `IRenderBackend`. Initialise a valid Vulkan context (instance, physical device, logical device, queues) without any rendering output. Add a build flag to select OpenGL or Vulkan at startup.

**Key changes**
- New `engine/renderer/Public/VulkanBackend.h` implementing `IRenderBackend`
- Instance creation with validation layers in debug builds
- Physical device selection (prefer AMD discrete GPU, check for required extensions)
- Logical device with graphics, compute, and transfer queues
- CMake option `ARCHE_RENDERER_BACKEND` (opengl | vulkan)
- Add VulkanMemoryAllocator (VMA) to `/tools` for buffer allocation

**Files:** `engine/renderer/Public/VulkanBackend.h`, `engine/renderer/CMakeLists.txt`, `app/viewer/main.cpp`

**Dependencies:** F-03 must be merged before this branch starts.

**Notes:** Use VMA (AMD's Vulkan Memory Allocator) from the start — it eliminates most manual memory management.

---

### F-07: Vulkan swapchain and frame loop
**Labels:** `phase:vulkan` `type:feature`

Swapchain creation, image views, render pass, framebuffers, and sync primitives. Implements `beginFrame()` / `endFrame()` in `VulkanBackend`. First milestone: a Vulkan-driven cleared frame appears in the viewport.

**Key changes**
- Swapchain creation and recreation on resize via `onResize()`
- Per-frame sync primitives (image available semaphore, render finished semaphore, in-flight fence)
- Minimal render pass (colour attachment, clear on load)
- `beginFrame()` acquires swapchain image, `endFrame()` submits and presents

**Files:** `engine/renderer/Public/VulkanBackend.h` and Private implementation

**Dependencies:** F-06.

---

### F-08: Vulkan geometry pass
**Labels:** `phase:vulkan` `type:feature`

Port the existing `GeometryPass` to Vulkan. Rasterized output should match the current OpenGL renderer.

**Key changes**
- Vertex/index buffer upload for all registered meshes
- Descriptor set layout for per-frame data (view/projection) and per-object data (model matrix)
- Push constants or UBO for transform data
- Port existing GLSL shaders to Vulkan-compatible GLSL
- Pipeline creation (vertex input, rasterisation state, depth test)
- Render entities from `RenderScene` (F-03)

**Files:** `engine/renderer/Public/VulkanBackend.h`, new `engine/renderer/Private/VulkanGeometryPass.cpp`

**Dependencies:** F-07, F-03.

---

### F-09: Vulkan ImGui integration
**Labels:** `phase:vulkan` `type:feature`

Port the editor UI to render via Vulkan using ImGui's Vulkan backend. Editor must remain fully functional after switching to `VulkanBackend`.

**Key changes**
- Initialise `imgui_impl_vulkan` alongside existing `imgui_impl_glfw`
- Descriptor pool for ImGui textures
- Render ImGui draw data in a separate render pass after geometry
- Remove `imgui_impl_opengl3` when Vulkan is the active backend

**Files:** `app/backends/imgui/ImGuiBackend.h`, `app/backends/imgui/GUIRunner.h`

**Dependencies:** F-08.

---

### F-10: Vulkan resource manager
**Labels:** `phase:vulkan` `type:feature`

Extend `ResourceRegistry` internals for the Vulkan backend. Manages GPU-side buffer and image lifetimes using VMA. Replaces ad-hoc buffer allocation spread across passes.

**Key changes**
- Buffer abstraction: staging upload, device-local buffer, mapped persistent buffer
- Image/sampler abstraction for texture resources
- Descriptor pool and set management
- Safe destruction (ensure GPU is idle before freeing)

**Files:** `engine/renderer/Public/ResourceRegistry.h`, new `engine/renderer/Private/VulkanResourceManager.cpp`

**Dependencies:** F-06 (VMA available).

---

## Phase 4 — GPU Compute

### F-11: Vulkan compute pipeline setup
**Labels:** `phase:gpu-compute` `type:feature`

Establish the Vulkan compute pipeline. Write a trivial compute shader (scale an array of floats), dispatch it, and read back results on the CPU. Validates the full compute path before it is needed for physics or ray tracing.

**Key changes**
- Compute pipeline creation (shader module, pipeline layout, descriptor set)
- Storage buffer creation, upload, dispatch, readback
- Synchronisation: buffer memory barriers between compute and subsequent stages
- Smoke test: upload 1024 floats, dispatch shader that doubles each, verify readback

**Files:** New `engine/renderer/Private/VulkanComputePipeline.cpp`

**Dependencies:** F-10 (resource manager for buffer allocation).

---

### F-12: GPU physics integration
**Labels:** `phase:gpu-compute` `type:feature`

Upload the SoA physics arrays (F-02) to GPU storage buffers. Write a compute shader that performs Euler integration, replacing `PhysicsSystem::update()` on the CPU.

**Key changes**
- Upload `positions[]`, `velocities[]`, `masses[]`, `isStatic[]`, `useGravity[]` to device-local SSBOs
- Compute shader: one thread per body, applies gravity, integrates velocity and position
- Gravity and deltaTime passed as push constants
- CPU readback of `positions[]` to sync `WorldSystem` entity transforms
- `PhysicsSystem::update()` dispatches compute shader instead of iterating CPU arrays

**Files:** `engine/physics/Public/PhysicsSystem.h`, new `assets/shaders/compute/physics_integrate.comp`

**Dependencies:** F-11, F-02.

---

### F-13: Persistent GPU buffers for scene data
**Labels:** `phase:gpu-compute` `type:performance`

Physics positions stay on the GPU between frames. The geometry pass reads positions directly from the GPU buffer, eliminating the per-frame CPU readback and re-upload.

**Key changes**
- Physics position SSBO shared between compute pass (write) and geometry pass (read)
- Buffer memory barrier between compute and vertex shader stages
- Per-frame upload only for changed data (new entities, property edits from UI)
- CPU readback retained only for editor gizmo/inspector display

**Files:** `engine/renderer/Private/VulkanGeometryPass.cpp`, `engine/physics/Public/PhysicsSystem.h`

**Dependencies:** F-12, F-08.

---

## Phase 5 — Ray Tracing Foundation

### F-14: Scene geometry in GPU buffers
**Labels:** `phase:rt-foundation` `type:feature`

All mesh vertex and index data available to compute shaders as device-local storage buffers. Triangle soup per mesh plus a per-instance transform buffer. This is the scene representation the ray tracer will traverse.

**Key changes**
- Upload all registered mesh vertex/index data to storage buffers on `initialise()`
- Per-instance buffer: transform matrix, mesh offset, material ID
- Buffer updated when entities are added, removed, or transformed
- Accessible to both the rasterisation pipeline and compute shaders

**Files:** `engine/renderer/Public/ResourceRegistry.h`, `engine/renderer/Private/VulkanResourceManager.cpp`

**Dependencies:** F-10, F-13.

---

### F-15: BVH construction
**Labels:** `phase:rt-foundation` `type:feature`

CPU-side build of an AABB bounding volume hierarchy over the scene geometry. Serialise the BVH node array to a GPU storage buffer.

**Key changes**
- BVH node struct: left/right child indices, AABB min/max, leaf triangle range
- Build algorithm: SAH split heuristic (start with midpoint split if SAH is too much upfront)
- Flat array serialisation — no pointers, GPU traversal requires index-based tree
- Upload BVH node buffer and triangle buffer to GPU
- Rebuild on scene change (entity added/removed/moved)

**Files:** New `engine/renderer/Public/BVH.h`, `engine/renderer/Private/BVH.cpp`

**Dependencies:** F-14.

**Notes:** BVH construction math transfers directly from a CPU path tracer. The GPU traversal shader uses iterative stack-based traversal, not recursion.

---

### F-16: Ray-AABB and ray-triangle intersection shaders
**Labels:** `phase:rt-foundation` `type:feature`

Implement ray-AABB slab test and ray-triangle Möller-Trumbore intersection in GLSL compute shaders. Validate with a debug visualisation (colour pixels by BVH traversal depth).

**Key changes**
- GLSL utility functions: `intersectAABB()`, `intersectTriangle()`, `traverseBVH()`
- Iterative BVH traversal with an explicit stack (no recursion in GLSL)
- Debug compute shader: outputs traversal depth as a heatmap to an output image
- Display the debug image in the viewport3D panel

**Files:** New `assets/shaders/compute/ray_utils.glsl`, `assets/shaders/compute/bvh_debug.comp`

**Dependencies:** F-15, F-11.

---

### F-17: Primary ray generation
**Labels:** `phase:rt-foundation` `type:feature`

Compute shader that generates one ray per pixel from the camera, traverses the BVH, and writes hit data (world position, normal, material ID) to a G-buffer. Display world-space normals as colour — first ray-traced image in the viewport.

**Key changes**
- Ray generation from camera view/projection matrices (inverse projection)
- BVH traversal per pixel, write closest hit to output image
- G-buffer layout: position (rgba32f), normal (rgba16f), material ID (r32ui)
- Display normal buffer as RGB in viewport3D

**Files:** New `assets/shaders/compute/primary_rays.comp`

**Dependencies:** F-16.

---

### F-18: Direct lighting pass
**Labels:** `phase:rt-foundation` `type:feature`

Shadow rays from each G-buffer hit point to the light source. Binary visibility test. Direct lighting output displayed in the viewport. Validates the full ray → intersect → shadow ray pipeline before adding bounces.

**Key changes**
- Compute shader reads G-buffer (F-17), spawns a shadow ray per pixel
- Shadow ray terminates early on any opaque hit (any-hit semantics)
- Lambertian direct lighting: `max(dot(N, L), 0) * lightColour * visibility`
- Light position/colour as push constants (single directional or point light)
- Output to a floating-point HDR image, tonemapped for display

**Files:** New `assets/shaders/compute/direct_lighting.comp`

**Dependencies:** F-17.

---

## Phase 6 — Path Tracer

### F-19: Path tracing kernel
**Labels:** `phase:path-tracer` `type:feature`

Extend the ray generation shader to full path tracing: iterative multi-bounce loop, cosine-weighted hemisphere sampling, throughput accumulation, Russian roulette termination. Start with 1 sample per pixel per frame and accumulate over time.

**Key changes**
- Iterative bounce loop (no recursion): sample new direction, multiply throughput by BRDF/pdf, trace next ray
- Cosine-weighted hemisphere sampling for diffuse surfaces
- Russian roulette: terminate paths with probability proportional to throughput
- Accumulation buffer (rgba32f): blend new sample with running average each frame
- Reset accumulation when camera moves or scene changes
- Per-pixel RNG state using PCG or xorshift stored in a persistent buffer

**Files:** New `assets/shaders/compute/path_trace.comp`

**Dependencies:** F-18.

**Notes:** Direct GPU port of a CPU path tracer — same mathematics, different execution model. Main GPU-specific challenge is managing per-pixel RNG state and the lack of recursion.

---

### F-20: GGX material model
**Labels:** `phase:path-tracer` `type:feature`

Implement GGX/Smith microfacet BRDF (roughness, metallic, F0) in the path tracing kernel. Importance sample the BRDF for efficient convergence. Existing `.mat` files map their properties to PBR roughness/metallic parameters.

**Key changes**
- GGX NDF, Smith geometry term, Schlick Fresnel approximation in GLSL
- BRDF importance sampling: sample the GGX NDF to generate reflected directions
- Material buffer on GPU: roughness, metallic, base colour, emissive per material ID
- Update material loader to parse PBR properties from `.mat` files
- Integrate into path tracing kernel replacing Lambertian diffuse

**Files:** New `assets/shaders/compute/brdf.glsl`, `engine/renderer/Public/MaterialLoader.h`

**Dependencies:** F-19.

---

### F-21: Temporal accumulation buffer
**Labels:** `phase:path-tracer` `type:feature`

Accumulate path tracing samples across frames into a persistent floating-point buffer. At rest, the image progressively converges to ground truth. Reset accumulation on camera movement or scene change.

**Key changes**
- Persistent HDR accumulation buffer (rgba32f), same resolution as viewport
- Blend factor: `colour = (prev * n + new) / (n + 1)` where n is the sample count
- Detect camera movement and scene changes, reset sample counter to 0
- Display sample count in the metrics panel
- Expose a max sample count setting (freeze accumulation when reached)

**Files:** `assets/shaders/compute/path_trace.comp`, `app/modules/metrics/`

**Dependencies:** F-19.

---

### F-22: Spatial denoiser
**Labels:** `phase:path-tracer` `type:feature`

Apply a spatial denoising filter to the noisy 1 spp path traced output to produce a clean image at interactive framerates. A-trous wavelet filter is the recommended starting point (5 passes, edge-stopping on normal and depth). SVGF can follow for temporal stability.

**Key changes**
- A-trous wavelet filter compute shader: 5 ping-pong passes with increasing kernel step width
- Edge-stopping functions based on normal similarity, depth similarity, luminance similarity
- Inputs: noisy colour buffer, G-buffer normals, G-buffer depth
- Expose filter strength and edge-stop weights as settings in the editor
- Toggle between raw path traced output and denoised output in the viewport

**Files:** New `assets/shaders/compute/atrous_filter.comp`

**Dependencies:** F-21, F-17 (G-buffer normals and depth needed for edge-stopping).

**Notes:** A-trous gives good results with low implementation cost. SVGF is the next step if temporal stability becomes a priority.
