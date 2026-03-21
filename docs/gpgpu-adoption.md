# GPGPU & HPC Adoption Notes

Notes on adopting GPU compute and HPC tooling in Arche. Covers OpenMP parallelism,
GPU offloading options for AMD hardware, and the architectural changes needed to get there.

---

## OpenMP — Where It Fits

OpenMP has two distinct roles relevant here:

### CPU Parallelism (entry point)

The physics update loop is embarrassingly parallel — each rigid body integration is
independent. This can be parallelised with minimal changes to `PhysicsSystem`:

```cpp
#pragma omp parallel for schedule(dynamic)
for (auto& entity : entities) {
    integrateRigidBody(entity, deltaTime);
}
```

This is the lowest-friction starting point and requires no architecture changes. It is
a good way to learn about data races, thread safety, and what "independent work" means
before touching GPU programming.

### GPU Offloading (OpenMP 4.0+ `target` directives)

OpenMP can also offload compute to GPUs via `#pragma omp target`. Clang with the
ROCm/AMDGPU backend supports this for AMD GPUs. The programming model remains pragma-based
but you must reason explicitly about host/device memory mapping and data transfer.

It is more portable than CUDA/HIP but less mature tooling-wise and gives less control
over GPU execution than dedicated APIs. Worth exploring as a comparison point after
learning HIP or Vulkan Compute.

---

## GPGPU API Options for AMD (No NVIDIA/CUDA)

| API | AMD Support | Portability | Notes |
|---|---|---|---|
| **HIP** | Native via ROCm | AMD + NVIDIA (hipcc) | AMD's CUDA equivalent, best AMD performance |
| **OpenCL** | Excellent | Very wide | Mature but verbose; older direction |
| **Vulkan Compute** | Excellent | AMD, NVIDIA, Intel | Same API as the rendering pipeline |
| **SYCL** | Via oneAPI/HIP backend | Wide | Higher-level C++ abstraction |
| **OpenMP offload** | Via Clang + ROCm | Good | Pragma-based, less control than HIP |

### Recommended Path: Vulkan Compute

Vulkan is the strongest fit for this project. Since Arche needs both a rendering API
and GPU compute, Vulkan provides both in one portable API — natively on AMD without
requiring a separate ROCm installation. A `VulkanBackend` would replace or sit alongside
`OpenGLBackend`, and the existing `IRenderBackend` abstraction is already in the right
place for this.

HIP is a valid alternative if the goal is pure compute work (e.g. physics-only) decoupled
from rendering, and it compiles for both AMD and NVIDIA via `hipcc`.

---

## Shaders vs. Kernels — Clarifying the Terminology

Shaders *are* GPU programs. The distinction being drawn here is between:

- **Rasterization pipeline** — vertex/fragment shaders driving traditional rendering
- **Compute shaders / kernels** — general-purpose GPU programs operating on arbitrary buffers

Compute shaders are the bridge between traditional rendering and GPGPU. In Vulkan (or
OpenGL 4.3+) they are written in GLSL, launched as dispatch calls, and operate on storage
buffers (SSBOs). The same GPU executes them — no separate API or hardware required.

For physics, the workflow becomes:
1. Physics state (positions, velocities, masses) lives in GPU-side storage buffers
2. A compute shader dispatches thousands of threads to integrate all bodies in parallel
3. The same buffer is consumed by the vertex shader for rendering — no CPU readback needed

This eliminates the CPU→GPU upload every frame, which becomes the bottleneck at scale.

---

## The Architectural Blocker: Entity Design

The current entity model will block GPU work if not addressed first:

```
IEntity (virtual interface)
  ↑
SphereEntity, CubeEntity, PlaneEntity  (heap-allocated, pointer-chasing)
```

GPUs require flat, contiguous data. Virtual dispatch, pointer indirection, and scattered
heap allocations are cache-hostile on CPU and simply don't map to GPU execution models.

### Required Change: Data-Oriented Layout (SoA)

Physics data needs to be separated from the entity hierarchy into flat arrays:

```cpp
struct PhysicsData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<glm::vec3> accelerations;
    std::vector<float>     masses;
    std::vector<bool>      isStatic;
};
```

This is a non-trivial change but the right time to make it — the entity system is still
small enough that a refactor is manageable. Delaying it means the codebase will grow
further around the current design and the refactor becomes more expensive.

The `IEntity` interface can remain for scene/editor purposes (transform, name, type),
but physics and render data should be decoupled into their own flat stores that can be
uploaded to GPU buffers directly.

---

## Suggested Adoption Path

### Step 1 — CPU OpenMP on `PhysicsSystem`

Add `#pragma omp parallel for` to the rigid body integration loop. Learn about data
races and thread safety. Verify correctness with the existing test suite.

No architecture changes required.

### Step 2 — Refactor Physics Data to SoA Layout

Decouple physics state from `IEntity` into flat arrays inside `PhysicsSystem`. This
is prerequisite for all GPU work downstream and is good design regardless of GPU.

The `WorldSystem` snapshot/restore feature needs to be updated to work with the new layout.

### Step 3 — Compute Shader Physics (OpenGL 4.3+)

Prototype GPU physics without leaving the current backend. Write an Euler integration
compute shader that operates on an SSBO containing the flat physics arrays. OpenGL 4.3
compute shaders are a low-friction entry point — same API, same window setup, no
Vulkan complexity yet.

### Step 4 — Evaluate Vulkan or HIP

Once compute shaders are understood conceptually:

- **Vulkan** — if the goal is integrated graphics + compute in one API with full control
  over the rendering pipeline. Higher upfront complexity, best long-term architecture.
- **HIP** — if the goal is GPU compute specifically (physics, simulation) with AMD-native
  performance and eventual NVIDIA portability via `hipcc`.

A new `VulkanBackend` implementing `IRenderBackend` would be the integration point for
Vulkan. `IRenderBackend` is already in the right place for this.

### Step 5 — OpenMP Offloading as a Comparison

Once HIP or Vulkan compute is working, revisit OpenMP `#pragma omp target` to understand
the tradeoffs between pragma-based and explicit GPU programming models.

---

## Hardware Note

ROCm support varies significantly across AMD GPU generations:

- **RDNA2 / RDNA3** (RX 6000 / RX 7000 series) — good ROCm support, HIP works well
- **RDNA1** (RX 5000 series) — partial ROCm support
- **older GCN** — limited or no ROCm support

If the target GPU does not have ROCm support, **Vulkan Compute is the better starting
path** as it works on essentially all AMD GPUs from GCN onwards without requiring ROCm.

Vulkan Compute also remains the most portable option across AMD, NVIDIA, and Intel
integrated graphics.

---

## What Is Already Well-Positioned

- `IRenderBackend` — clean abstraction point for adding a `VulkanBackend`
- `ISubsystem` — `PhysicsSystem` can be evolved internally without disrupting the rest
- `ResourceRegistry` — can be extended to manage GPU buffers alongside meshes/materials
- The engine/app split means GPU work in `PhysicsSystem` and `RenderingSystem` stays
  decoupled from the editor UI
