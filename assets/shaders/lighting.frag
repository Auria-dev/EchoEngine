#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gARM;

struct Light { // multiply by view matrix before hand
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    float constant;
    float linear;
    float quadratic;
};

const float PI = 3.14159265359;

float D_GGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float G_SchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx_V = G_SchlickGGX(NdotV, roughness);
    float ggx_L = G_SchlickGGX(NdotL, roughness);
    return ggx_V * ggx_L;
}

vec3 F_Schlick(float cosTheta, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    vec3 ARM = texture(gARM, TexCoords).rgb;
    
    float roughness = ARM.g;
    float metallic = ARM.b;

    Light light = Light(
        vec3(0.0, 0.0, 0.0),
        vec3(0.1, 0.1, 0.1),
        vec3(0.8, 0.8, 0.8),
        1.0, 0.09, 0.032
    );

    vec3 V = normalize(-fragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    vec3 radiance = light.diffuse * attenuation;

    float NDF = D_GGX(normal, H, roughness);
    float G   = G_Smith(normal, V, L, roughness);
    vec3 F    = F_Schlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = light.ambient * albedo;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}