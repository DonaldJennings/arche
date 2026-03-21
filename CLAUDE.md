# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure build
cmake -S . -B build

# Build everything (app + tests)
cmake --build build

# Run the editor application
./build/bin/arche-gui-viewer

# Run all tests
./build/bin/arche-engine-tests

# Run a single test (Catch2 filter syntax)
./build/bin/arche-engine-tests "[physics]"
./build/bin/arche-engine-tests "Gravity is applied"

# Format code
cmake --build build --target clang_format
```

**Requirements:** CMake 3.15+, C++17 compiler, OpenGL 3.3+. All third-party dependencies (GLFW, Glad, GLM, ImGui, Catch2) are in `/tools` as CMake subprojects.

Binaries output to `${CMAKE_BINARY_DIR}/bin`. A post-build step copies `/assets` to the executable directory.

## Architecture

Arche is a modular physics and rendering engine with an ImGui-based editor. The codebase is split into two layers:

- **`/engine`** — Core engine with no UI dependencies (physics, rendering, scene, core services)
- **`/app`** — Application/editor layer (ImGui panels, GLFW windowing, editor session)

### Engine Core (`/engine/core`)

`EngineCore` is the central coordinator. It owns:
- `LoggingService` — Multi-sink logging; `ILogSink` interface allows routing to console or GUI
- `TimingService` — Frame timing and delta time
- `GlobalSettings` — `WorldSettings` and `RenderSettings` structs
- Three subsystems registered via `ISubsystem` interface: `WorldSystem`, `PhysicsSystem`, `RenderingSystem`

All subsystems implement `ISubsystem` (initialise / update / shutdown). The main tick loop calls `engineCore->tick()` which propagates to all subsystems in order.

Simulation state machine: `Idle → Running ↔ Paused`. State transitions are driven by the editor.

### Scene (`/engine/scene`)

`WorldSystem` manages entities and supports state snapshots (save/restore for simulation reset). Entities are typed (`SphereEntity`, `CubeEntity`, `PlaneEntity`) and implement `IEntity`, which combines transform, optional physics (`RigidBody`), and optional render data in a single interface. There is no ECS; entities are monolithic.

### Physics (`/engine/physics`)

`PhysicsSystem` applies gravity and Euler-integrates rigid body forces into velocities/positions each tick. Collision shapes (`SphereCollider`, `CubeCollider`, `PlaneCollider`) implement `ICollider` but collision *resolution* is not yet implemented (MVP phase).

### Rendering (`/engine/renderer`)

`RenderingSystem` owns a `ResourceRegistry` (meshes, materials, shaders) and dispatches to render passes via `IRenderPass`. Passes: `GeometryPass`, `ShadowPass`, `SkyPass`, `DebugPass`. The graphics backend is abstracted by `IRenderBackend`; `OpenGLBackend` is the only current implementation (OpenGL 3.3+).

Shaders live in `/assets/shaders/` (blinn-phong, pbr, flat, depth-map, debug-lines). Materials are defined as `.mat` files in `/assets/materials/`.

### Application Layer (`/app`)

`EditorSession` wraps `EngineCore` for use by the editor. GUI panels implement `IPanel` and are registered in `PanelRegistry`. Each module under `/app/modules/` is a self-contained panel (viewport3D, world-properties, log, metrics, material-browser, shader-browser, entity-inspector, gizmo, dockspace).

`GUILogSink` bridges `LoggingService` to the GUI log panel by implementing `ILogSink`.

The main entry point (`app/viewer/main.cpp`) shows the full initialization order: GLFW → OpenGL context → EngineCore → subsystems → GUI panels → main loop.

## Code Conventions

- **Namespaces:** `Arche::Core`, `Arche::Scene`, `Arche::Render`, `Arche::Physics`, `Arche::GUI`
- **Headers split:** `Public/` (API surface) and `Private/` within each subsystem directory
- **Memory:** `std::shared_ptr` for cross-subsystem ownership; subsystem dependencies injected via constructor
- **Tests:** Catch2 BDD style (SCENARIO / GIVEN / WHEN / THEN). Test files are in `/tests/`.
- **Performance measurement:** `ScopedTimer` (RAII) in `engine/core`
