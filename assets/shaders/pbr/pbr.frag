#version 460 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 fragColor;

// Material parameters
uniform vec3  uAlbedo;        // base color without metal tinting
uniform float uMetallic;
uniform float uRoughness;
uniform float uAO;            // ambient occlusion

// Lighting
uniform vec3 uLightPos;
uniform vec3 uLightColor;

uniform vec3 uCameraPos;

const float PI = 3.14159265359;

// -----------------------------------------------------------
// Helper functions
// -----------------------------------------------------------

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = (NdotV * (1.0 - k) + k);

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// -----------------------------------------------------------
// Main PBR evaluation
// -----------------------------------------------------------

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 L = normalize(uLightPos - vWorldPos);
    vec3 H = normalize(V + L);

    float distance    = length(uLightPos - vWorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = uLightColor * attenuation;

    // Base reflectance
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, uAlbedo, uMetallic);

    // Cook–Torrance
    float NDF = DistributionGGX(N, H, uRoughness);
    float G   = GeometrySmith(N, V, L, uRoughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator   = NDF * G * F;
    float denominator = 4.0 *
        max(dot(N, V), 0.0) *
        max(dot(N, L), 0.0) + 0.0001;

    vec3 specular = numerator / denominator;

    // kS + kD
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - uMetallic;

    // Lambertian diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * uAlbedo / PI;

    vec3 color = (diffuse + specular) * radiance * NdotL;

    // Ambient approximation
    vec3 ambient = uAlbedo * uAO;

    color += ambient;

    // Final output
    fragColor = vec4(color, 1.0);
}
