#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 0, binding = 0) uniform ViewProjUBO {
    mat4 view;
    mat4 proj;
} ubo;

// Push constants shared with fragment shader.
// Total layout: 128 bytes (guaranteed minimum).
layout(push_constant) uniform PC {
    mat4 model;         // offset   0, 64 B — vertex only
    vec4 albedo;        // offset  64, 16 B — fragment only
    vec4 lightDir;      // offset  80, 16 B — fragment only (w unused)
    vec4 lightColor;    // offset  96, 16 B — fragment only (w unused)
    vec4 cameraPos;     // offset 112, 16 B — fragment only (w unused)
};

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;

void main() {
    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;
    fragNormal    = mat3(transpose(inverse(model))) * inNormal;
    gl_Position   = ubo.proj * ubo.view * worldPos;
}
