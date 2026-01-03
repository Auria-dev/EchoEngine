#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gARM;
uniform sampler2D gSSAO;

#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 8

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    
    float constant;
    float innerCutoff;
    float outerCutoff;
};

uniform DirectionalLight uDirLights[MAX_DIR_LIGHTS];
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform SpotLight uSpotLights[MAX_SPOT_LIGHTS];

uniform int uDirLightCount;
uniform int uPointLightCount;
uniform int uSpotLightCount;

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

vec3 CalculatePBRLighting(vec3 L, vec3 V, vec3 N, vec3 radiance, vec3 albedo, float roughness, float metallic, vec3 F0) {
    vec3 H = normalize(V + L);
    
    float NDF = D_GGX(N, H, roughness);
    float G   = G_Smith(N, V, L, roughness);
    vec3 F    = F_Schlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() {
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal  = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo  = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    vec3 ARM     = texture(gARM, TexCoords).rgb;
    float SSAO   = texture(gSSAO, TexCoords).r;

    float roughness = ARM.g;
    float metallic  = ARM.b;
    float ao        = ARM.r;

    vec3 V = normalize(-fragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for(int i = 0; i < uDirLightCount; ++i) {
        vec3 L = normalize(-uDirLights[i].direction);
        vec3 radiance = uDirLights[i].color * uDirLights[i].intensity;
        Lo += CalculatePBRLighting(L, V, normal, radiance, albedo, roughness, metallic, F0);
    }

    for(int i = 0; i < uPointLightCount; ++i) {
        vec3 L = normalize(uPointLights[i].position - fragPos);
        float distance = length(uPointLights[i].position - fragPos);
        float attenuation = 1.0 / (uPointLights[i].constant + uPointLights[i].linear * distance + uPointLights[i].quadratic * (distance * distance));
        vec3 radiance = uPointLights[i].color * uPointLights[i].intensity * attenuation;
        Lo += CalculatePBRLighting(L, V, normal, radiance, albedo, roughness, metallic, F0);
    }

    for(int i = 0; i < uSpotLightCount; ++i) {
        vec3 L = normalize(uSpotLights[i].position - fragPos);
        float distance = length(uSpotLights[i].position - fragPos);
        
        float attenuation = 1.0 / (uSpotLights[i].constant + 0.09 * distance + 0.032 * (distance * distance));
        float theta = dot(L, normalize(-uSpotLights[i].direction)); 
        float epsilon = uSpotLights[i].innerCutoff - uSpotLights[i].outerCutoff;
        float intensity = clamp((theta - uSpotLights[i].outerCutoff) / epsilon, 0.0, 1.0); 
        vec3 radiance = uSpotLights[i].color * uSpotLights[i].intensity * attenuation * intensity;
        Lo += CalculatePBRLighting(L, V, normal, radiance, albedo, roughness, metallic, F0);
    }

    vec3 ambient = vec3(0.05) * albedo * ao * SSAO;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}