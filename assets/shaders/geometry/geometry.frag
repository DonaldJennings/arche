#version 450

layout(location = 0) in  vec3 fragNormal;
layout(location = 1) in  vec3 fragWorldPos;
layout(location = 0) out vec4 outColor;

void main() {
    vec3  N         = normalize(fragNormal);
    vec3  lightDir  = normalize(vec3(0.4, 1.0, 0.6));
    float diff      = max(dot(N, lightDir), 0.0);
    vec3  baseColor = vec3(0.6, 0.7, 0.8);
    outColor = vec4(baseColor * (0.2 + 0.8 * diff), 1.0);
}
