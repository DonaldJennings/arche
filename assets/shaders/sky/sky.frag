#version 460 core
in vec3 vDir;
out vec4 fragColor;

uniform vec3 uSunDir;
uniform vec3 uSunColor;

vec3 skyColor(vec3 d, vec3 sunDir)
{
    float t = clamp(d.y * 0.5 + 0.5, 0.0, 1.0);  
    vec3 zenith = vec3(0.1, 0.25, 0.55);
    vec3 horizon = vec3(0.8, 0.9, 1.0);

    float sunAmount = max(dot(d, sunDir), 0.0);
    float sunGlow = pow(sunAmount, 1024.0);

    return mix(horizon, zenith, t) + sunGlow * uSunColor;
}

void main()
{
    vec3 col = skyColor(normalize(vDir), normalize(uSunDir));
    fragColor = vec4(col, 1.0);
}
