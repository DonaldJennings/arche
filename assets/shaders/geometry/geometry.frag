#version 450

layout(location = 0) in  vec3 fragNormal;
layout(location = 1) in  vec3 fragWorldPos;
layout(location = 0) out vec4 outColor;

// Push constants — must match geometry.vert layout exactly.
layout(push_constant) uniform PC {
    mat4 model;         // offset   0 (unused in frag)
    vec4 albedo;        // offset  64
    vec4 lightDir;      // offset  80 (w unused)
    vec4 lightColor;    // offset  96 (w unused)
    vec4 cameraPos;     // offset 112 (w unused)
};

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir.xyz);
    vec3 V = normalize(cameraPos.xyz - fragWorldPos);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 ambient  = 0.05 * albedo.rgb;
    vec3 diffuse  = albedo.rgb * NdotL;
    vec3 specular = vec3(pow(NdotH, 32.0)) * 0.3;

    vec3 color = ambient + (diffuse + specular) * lightColor.rgb;

    outColor = vec4(color, albedo.a);
}
