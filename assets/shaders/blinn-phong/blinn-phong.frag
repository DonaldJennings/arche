#version 460 core

in vec3 vWorldPos;
in vec3 vNormal;

out vec4 fragColor;

uniform vec4 uBaseColor;   // diffuse color
uniform float uShininess;  // specular shininess exponent

uniform vec3 uLightDir;    // world-space directional light direction (normalized)
uniform vec3 uLightColor;  // RGB intensity

uniform vec3 uCameraPos;   // view position

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);

    // Blinn–Phong half vector
    vec3 H = normalize(L + V);

    // Diffuse term
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = uBaseColor.rgb * NdotL;

    // Specular (Blinn–Phong)
    float spec = pow(max(dot(N, H), 0.0), uShininess);
    vec3 specular = vec3(spec);

    // Ambient term
    vec3 ambient = 0.05 * uBaseColor.rgb;

    vec3 finalColor = ambient + (diffuse + specular) * uLightColor;

    fragColor = vec4(finalColor, uBaseColor.a);
}
