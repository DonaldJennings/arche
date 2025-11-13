#version 460 core
layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;
uniform float uPointSize;
void main(){
    gl_Position = uProj * uView * uModel * vec4(inPosition, 1.0);
    gl_PointSize = uPointSize;
}
