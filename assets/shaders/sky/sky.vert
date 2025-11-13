#version 460 core
layout(location=0) in vec3 inPosition;

out vec3 vDir;

uniform mat4 uInvViewProj;

void main()
{
    // Convert cube-vertex from [-1..1] to direction in world space
    vec4 p = vec4(inPosition, 1.0);
    vec4 world = uInvViewProj * p;
    vDir = normalize(world.xyz / world.w);

    gl_Position = p;
}
