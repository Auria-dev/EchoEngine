#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gDepth;

uniform sampler2D uShadowMap;
uniform mat4 uLightProj;

uniform vec3 viewPos;
uniform vec3 viewDir;
uniform mat4 uInvViewProj;
uniform vec3 uLightDir;
uniform vec3 uLightColor;

const int NB_STEPS = 64;
const float DENSITY = 0.03;
const vec3 FOG_COLOR = vec3(1.0);

float InterleavedGradientNoise(vec2 pixel) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(pixel, magic.xy)));
}

float CalculateShadow(vec3 worldPos)
{
    vec4 fragPosLightSpace = uLightProj * vec4(worldPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0 || projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
    {
        return 1.0;
    }

    float closestDepth = texture(uShadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    float bias = 0.005; 
    return currentDepth - bias > closestDepth ? 0.0 : 1.0;;
}

vec2 rayAABB(vec3 ro, vec3 rd, vec3 AABBMin, vec3 AABBMax)
{
    vec3 t0 = (AABBMin - ro) / rd;
    vec3 t1 = (AABBMax - ro) / rd;
    vec3 tmin = min(t0,t1);
    vec3 tmax = max(t0,t1);

    float distA = max(max(tmin.x, tmin.y), tmin.z);
    float distB = min(tmax.x, min(tmax.y, tmax.z));

    float distToBox = max(0, distA);
    float distInsideBox = max(0, distB - distToBox);
    return vec2(distToBox, distInsideBox);
}

vec3 GetWorldPosFromDepth(float depth, vec2 uv)
{
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uInvViewProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    return viewSpacePosition.xyz;
}

// Phase function (Henyey-Greenstein)
// Controls how much light scatters toward the camera
float PhaseFunction(vec3 viewDir, vec3 lightDir, float g)
{
    float cosTheta = dot(viewDir, -lightDir);
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
    // return 1.0/(4.0*3.14);
}

void main()
{
    float dither = InterleavedGradientNoise(gl_FragCoord.xy);

    vec3 AABBMin = viewPos + vec3(-50, -50, -50);
    vec3 AABBMax = viewPos + vec3(50, 50, 50);

    float depthValue = texture(gDepth, TexCoords).r;
    vec3 worldPosGeometry = GetWorldPosFromDepth(depthValue, TexCoords);
    vec3 rayDir = normalize(worldPosGeometry - viewPos);
    float distToGeometry = length(worldPosGeometry - viewPos);

    vec2 rayBoxInfo = rayAABB(viewPos, rayDir, AABBMin, AABBMax);
    float distToBoxEntry = rayBoxInfo.x;
    float distInsideBox = rayBoxInfo.y;

    float distToBoxExit = distToBoxEntry + distInsideBox;
    float marchEnd = min(distToBoxExit, distToGeometry);
    
    if (distToBoxEntry > marchEnd || distInsideBox <= 0.0) {
        discard;
    }

    float totalDistanceToMarch = marchEnd - distToBoxEntry;
    float stepSize = totalDistanceToMarch / float(NB_STEPS);

    // vec3 currentPos = viewPos + rayDir * distToBoxEntry;
    vec3 currentPos = viewPos + rayDir * (distToBoxEntry + stepSize * dither);
    
    float transmittance = 1.0;
    vec3 accumulatedColor = vec3(0.0);

    // base scattering (S) = Incoming Light * Phase * Fog Color * Density
    float g = 0.6; // isotropic scattering
    float phase = PhaseFunction(rayDir, uLightDir, g);
    vec3 baseS = uLightColor * phase * FOG_COLOR * DENSITY;
    float sigmaE = DENSITY; // Extinction coefficient

    for(int i = 0; i < NB_STEPS; i++)
    {
        // TODO: sample noise here
        vec3 samplePos = currentPos + rayDir * (stepSize * 0.5);
        
        float visibility = CalculateShadow(samplePos);
        vec3 currentS = baseS * visibility;

        // Scattering Integration = (S - S * exp(-sigma * d)) / sigma
        vec3 Sint = (currentS - currentS * exp(-sigmaE * stepSize)) / sigmaE;
        
        accumulatedColor += transmittance * Sint;
        transmittance *= exp(-sigmaE * stepSize); //Beer's Law

        currentPos += rayDir * stepSize;
        if(transmittance < 0.01) break;

    }

    FragColor = vec4(accumulatedColor, 1.0 - transmittance);
}