#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vColor;
out vec3 vWorldPos; // Pass world position to fragment shader

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

void main() {
    vColor = aColor;
    // Assuming uModel is identity for the grid, aPos is world position
    vWorldPos = aPos;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}