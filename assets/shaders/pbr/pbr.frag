#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
in vec4 vLightSpacePos;

out vec4 FragColor;

// Material
uniform vec3  uAlbedo    = vec3(1.0);
uniform float uMetallic  = 0.1;
uniform float uRoughness = 0.5;
uniform float uAO        = 1.0;

// Camera
uniform vec3 uCameraPos = vec3(0.0, 0.0, 5.0);

// Light (fallbacks provided)
uniform vec3 uLightDir   = normalize(vec3(-1.0, -1.0, -1.0));
uniform vec3 uLightColor = vec3(10.0);

// Shadow map (optional)
uniform sampler2D uShadowMap;

// Toggle — if not set, defaults to 0
uniform bool uHasShadowMap = false;

// ---------------------------
// Shadow sampling helper
// ---------------------------
float computeShadow(vec4 lightSpacePos)
{
    if (!uHasShadowMap)
        return 0.0; // No shadowing

    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Outside shadow texture
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.0015;
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}

// ---------------------------
// PBR helpers
// ---------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;

    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r*r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness)
         * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---------------------------
// Main
// ---------------------------
void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 L = normalize(uLightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);

    // Base reflectance
    vec3 F0 = mix(vec3(0.04), uAlbedo, uMetallic);

    // Cook–Torrance PBR
    float D = DistributionGGX(N, H, uRoughness);
    float G = GeometrySmith(N, V, L, uRoughness);
    vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  numerator = D * G * F;
    float denom = 4.0 * max(dot(N,V),0.0) * NdotL + 0.001;
    vec3  specular = numerator / denom;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - uMetallic);

    vec3 diffuse = uAlbedo / 3.14159265;
    vec3 radiance = uLightColor;

    vec3 directLighting = (kD * diffuse + specular) * radiance * NdotL;

    // Shadow term
    float shadow = computeShadow(vLightSpacePos);
    directLighting *= (1.0 - shadow);

    // Ambient
    vec3 ambient = vec3(0.03) * uAlbedo * uAO;

    vec3 color = ambient + directLighting;

    // Basic tonemap + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
