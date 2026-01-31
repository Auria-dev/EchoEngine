#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gARM;
uniform sampler2D gSSAO;

// uniform samplerCube irradianceMap;
// uniform samplerCube prefilterMap;
// uniform sampler2D brdfLUT;

uniform sampler2DArrayShadow uShadowMap;
uniform int uCascadeCount;
uniform float uCascadePlaneDistances[16];
uniform mat4 uCascadeMatrices[16];

// uniform sampler2D uPrefilteredMap;
uniform sampler2D uTransmittanceLUT;
uniform samplerCube uSkyProbe;

#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 8

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
    
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
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

uniform vec3  uSunDirection;
uniform vec3  uSunColor;
uniform float uSunIntensity;

uniform int uPointLightCount;
uniform int uSpotLightCount;
uniform mat4 uInverseView;
uniform mat4 uView;

const float PI = 3.14159265359;

const float RGround = 6360.0;
const float RTop    = 6460.0;

vec3 GetTransmittance(vec3 worldPos, vec3 lightDir)
{
    vec3 planetCenter = vec3(0.0, -RGround, 0.0);
    float altitude = length(worldPos - planetCenter) - RGround;
    float v = clamp(altitude / (RTop - RGround), 0.0, 1.0);
    float cosTheta = dot(normalize(worldPos - planetCenter), lightDir);
    float u = clamp((cosTheta + 1.0) / 2.0, 0.0, 1.0);
    return texture(uTransmittanceLUT, vec2(u, v)).rgb;
}

float D_GGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float G_SchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx_V = G_SchlickGGX(NdotV, roughness);
    float ggx_L = G_SchlickGGX(NdotL, roughness);
    return ggx_V * ggx_L;
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 F_SchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalculatePBRLighting(vec3 L, vec3 V, vec3 N, vec3 radiance, vec3 albedo, float roughness, float metallic, vec3 F0)
{
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

float SampleShadowMap(int layer, vec3 fragPosWorld, vec3 normal, vec3 lightDir)
{
    vec4 fragPosLightSpace = uCascadeMatrices[layer] * vec4(fragPosWorld, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    if (layer == uCascadeCount - 1) bias *= 0.5;
    
    float currentDepth = projCoords.z;
    
    float shadow = 0.0;    
    vec2 texSize = textureSize(uShadowMap, 0).xy;
    vec2 texelSize = 1.0 / texSize;
    
    int sampleRadius = 1; 
    for(int y = -sampleRadius; y <= sampleRadius; y++)
    {
        for(int x = -sampleRadius; x <= sampleRadius; x++)
        {
            vec4 coords = vec4(projCoords.xy + vec2(x, y) * texelSize, layer, currentDepth - bias);
            shadow += texture(uShadowMap, coords); 
        }    
    }
    
    shadow /= pow((sampleRadius * 2 + 1), 2);
    
    return 1.0 - shadow; 
}

float ShadowCalculation(vec3 fragPosWorld, vec3 normal, vec3 lightDir)
{
    vec4 fragPosView = uView * vec4(fragPosWorld, 1.0);
    float depthValue = abs(fragPosView.z);

    int layer = -1;
    for (int i = 0; i < uCascadeCount; ++i)
    {
        if (depthValue < uCascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1) layer = uCascadeCount - 1;

    float shadow = SampleShadowMap(layer, fragPosWorld, normal, lightDir);
    float blendDistance = 20.0; 
    
    float nextSplitDistance = uCascadePlaneDistances[layer];
    float distToNextSplit = nextSplitDistance - depthValue;

    if (distToNextSplit < blendDistance && layer < uCascadeCount - 1)
    {
        float blendFactor = (blendDistance - distToNextSplit) / blendDistance;
        float nextShadow = SampleShadowMap(layer + 1, fragPosWorld, normal, lightDir);
        shadow = mix(shadow, nextShadow, blendFactor);
    }

    return shadow;
}

void main()
{
    vec3 fragPosView = texture(gPosition, TexCoords).rgb;
    vec3 normalView  = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo      = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    vec3 ARM         = texture(gARM, TexCoords).rgb;
    float SSAO       = texture(gSSAO, TexCoords).r;

    float roughness = ARM.g;
    float metallic  = ARM.b;
    float ao        = ARM.r;

    vec4 worldPosRaw = uInverseView * vec4(fragPosView, 1.0);
    vec3 worldPos    = worldPosRaw.xyz;
    
    vec3 N = normalize(vec3(uInverseView * vec4(normalView, 0.0))); 
    
    vec3 camPos = vec3(uInverseView[3]); 
    vec3 V = normalize(camPos - worldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 fragPosKM = (worldPos * 0.001) + vec3(0.0, RGround, 0.0);

    vec3 sunL = normalize(-uSunDirection);
    vec3 sunTransmittance = GetTransmittance(fragPosKM, sunL);
    float shadow = ShadowCalculation(worldPos, N, sunL);
    vec3 sunRadiance = uSunColor * uSunIntensity * sunTransmittance;
    vec3 lightContribution = CalculatePBRLighting(sunL, V, N, sunRadiance, albedo, roughness, metallic, F0);
    Lo += (1.0 - shadow) * lightContribution;

    for(int i = 0; i < uPointLightCount; ++i)
    {
        vec3 L = normalize(uPointLights[i].position - worldPos);
        float distance = length(uPointLights[i].position - worldPos);
        float attenuation = 1.0 / (uPointLights[i].constant + uPointLights[i].linear * distance + uPointLights[i].quadratic * (distance * distance));
        vec3 radiance = uPointLights[i].color * uPointLights[i].intensity * attenuation;
        Lo += CalculatePBRLighting(L, V, N, radiance, albedo, roughness, metallic, F0);
    }

    for(int i = 0; i < uSpotLightCount; ++i)
    {
        vec3 L = normalize(uSpotLights[i].position - worldPos);
        float distance = length(uSpotLights[i].position - worldPos);
        
        float attenuation = 1.0 / (uSpotLights[i].constant + 0.09 * distance + 0.032 * (distance * distance));
        float theta = dot(L, normalize(-uSpotLights[i].direction)); 
        float epsilon = uSpotLights[i].innerCutoff - uSpotLights[i].outerCutoff;
        float intensity = clamp((theta - uSpotLights[i].outerCutoff) / epsilon, 0.0, 1.0); 
        vec3 radiance = uSpotLights[i].color * uSpotLights[i].intensity * attenuation * intensity;
        Lo += CalculatePBRLighting(L, V, N, radiance, albedo, roughness, metallic, F0);
    }

    vec3 kS = F_SchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    vec3 worldNormal = normalize(vec3(uInverseView * vec4(N, 0.0)));
    vec3 irradiance = textureLod(uSkyProbe, N, 5.0).rgb;
    vec3 diffuse = irradiance * albedo;

    vec3 ambient = (kD * diffuse) * ao * SSAO * mix(0.7, 1.0, (1.0-shadow));
    vec3 color = ambient + Lo;

    FragColor = vec4(color, 1.0);

    // vec3 R = reflect(normalize(worldPos - camPos), N);
    // vec3 envColor = textureLod(uSkyProbe, R, 0.0).rgb;
    // FragColor = vec4(envColor, 1.0);

    // CSM debugging    
    // vec4 fragPosViewDebug = uView * vec4(worldPos, 1.0);
    // float depthValueDebug = abs(fragPosViewDebug.z);

    // int debugLayer = -1;
    // for (int i = 0; i < uCascadeCount; ++i)
    // {
    //     if (depthValueDebug < uCascadePlaneDistances[i])
    //     {
    //         debugLayer = i;
    //         break;
    //     }
    // }
    // if (debugLayer == -1) debugLayer = uCascadeCount - 1;

    // vec3 colors[5];
    // colors[0] = vec3(1.0, 0.2, 0.2); // Red
    // colors[1] = vec3(0.2, 1.0, 0.2); // Green
    // colors[2] = vec3(0.2, 0.2, 1.0); // Blue
    // colors[3] = vec3(1.0, 1.0, 0.2); // Yellow
    // colors[4] = vec3(0.2, 1.0, 1.0); // Cyan
    
    // vec3 debugColor = colors[debugLayer];
    // float debugBlendDist = 20.0; 
    // float distToNextSplit = uCascadePlaneDistances[debugLayer] - depthValueDebug;

    // if (distToNextSplit < debugBlendDist && debugLayer < uCascadeCount - 1)
    // {
    //     float blendFactor = (debugBlendDist - distToNextSplit) / debugBlendDist;
    //     debugColor = mix(colors[debugLayer], colors[debugLayer + 1], blendFactor);
    // }


    // if (TexCoords.x < 0.5) FragColor = vec4(debugColor * (1.0 - (shadow * 0.5)), 1.0); 
}
