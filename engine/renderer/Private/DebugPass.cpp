#include "DebugPass.h"

void Arche::Render::DebugPass::render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                                      ResourceRegistry &resources, const Core::RenderSettings &settings) {

    // Bind the dedicated debug line shader
    if (auto debugShader = resources.getShader("DebugLines")) {
        backend.setShader(debugShader);
        backend.setUniformMat4("uModel", glm::identity<glm::mat4>());
        backend.setUniformMat4("uView", view.viewMatrix);
        backend.setUniformMat4("uProj", view.projectionMatrix);
        backend.setUniformVec3("uCameraPos", view.cameraPosition); // Pass camera position
    }

    // ------------------------------------------------------------
    // Grid
    // ------------------------------------------------------------
    if (settings.showGrid)
        drawGrid(backend, view.cameraPosition, resources);

    // Restore
    backend.setWireframe(false);
}

void Arche::Render::DebugPass::drawGrid(IRenderBackend &backend, const glm::vec3 &cameraPosition,
                                        ResourceRegistry &resources) {

    const int lineCount = 101; // Number of lines to draw in each direction (must be odd)
    const int halfLineCount = lineCount / 2;
    const float spacing = 1.0f;
    const float gridSize = (lineCount - 1) * spacing;
    const float halfGridSize = gridSize / 2.0f;

    backend.beginDebugLines();

    // Snap the grid origin to the nearest grid line intersection to prevent shimmering when moving
    glm::vec3 origin(
        std::round(cameraPosition.x / spacing) * spacing,
        0.0f,
        std::round(cameraPosition.z / spacing) * spacing
    );

    for (int i = -halfLineCount; i <= halfLineCount; ++i) {
        float p = i * spacing;

        // Z-aligned grid lines
        backend.drawDebugLine(origin + glm::vec3(p, 0.0f, -halfGridSize),
                              origin + glm::vec3(p, 0.0f, halfGridSize), glm::vec3(0.3f));

        // X-aligned grid lines
        backend.drawDebugLine(origin + glm::vec3(-halfGridSize, 0.0f, p),
                              origin + glm::vec3(halfGridSize, 0.0f, p), glm::vec3(0.3f));
    }

    backend.endDebugLines();
}
