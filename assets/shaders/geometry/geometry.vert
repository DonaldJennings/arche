#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 0, binding = 0) uniform ViewProjUBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PC {
    mat4 model;
};

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;

void main() {
    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;
    fragNormal    = mat3(transpose(inverse(model))) * inNormal;
    gl_Position   = ubo.proj * ubo.view * worldPos;
}
