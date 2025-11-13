#version 460 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main()
{
    vec4 worldPos = uModel * vec4(inPosition, 1.0);

    vWorldPos = worldPos.xyz;
    vNormal   = normalize(mat3(uModel) * inNormal);
    vUV       = inUV;

    gl_Position = uProj * uView * worldPos;
}
