#version 330 core
in vec3 vColor;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 uCameraPos;

void main() {
    // Fade based on distance from camera
    float camDist = distance(vWorldPos, uCameraPos);
    float camFade = 1.0 - smoothstep(45.0, 60.0, camDist);

    FragColor = vec4(vColor, camFade);
}