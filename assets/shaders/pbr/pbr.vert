#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out vec4 vLightSpacePos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

// Optional — if not supplied, defaults to identity
uniform mat4 uLightSpaceMatrix = mat4(1.0);

void main()
{
    vec4 worldPos = uModel * vec4(inPosition, 1.0);

    vWorldPos = worldPos.xyz;
    vNormal   = mat3(transpose(inverse(uModel))) * inNormal;
    vUV       = inUV;

    // For shadow mapping (safe if identity)
    vLightSpacePos = uLightSpaceMatrix * worldPos;

    gl_Position = uProj * uView * worldPos;
}
