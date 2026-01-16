#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gDepth;

uniform vec3 viewPos;
uniform vec3 viewDir;
uniform mat4 uInvViewProj;
uniform vec3 uLightDir;
uniform vec3 uLightColor;

const int NB_STEPS = 64;
const float DENSITY = 0.1;
const vec3 FOG_COLOR = vec3(1.0, 1.0, 1.0);

vec2 ray_aabb(vec3 ro, vec3 rd, vec3 aabbMin, vec3 aabbMax)
{
    vec3 t0 = (aabbMin - ro) / rd;
    vec3 t1 = (aabbMax - ro) / rd;
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
    float cosTheta = dot(viewDir, lightDir);
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
    // return 1.0/(4.0*3.14);
}

void main()
{
    const vec3 aabbMin = vec3(-5, 10, -5);
    const vec3 aabbMax = vec3( 8, 20,  8);

    float depthValue = texture(gDepth, TexCoords).r;
    vec3 worldPosGeometry = GetWorldPosFromDepth(depthValue, TexCoords);
    vec3 rayDir = normalize(worldPosGeometry - viewPos);
    float distToGeometry = length(worldPosGeometry - viewPos);

    vec2 rayBoxInfo = ray_aabb(viewPos, rayDir, aabbMin, aabbMax);
    float distToBoxEntry = rayBoxInfo.x;
    float distInsideBox = rayBoxInfo.y;

    float distToBoxExit = distToBoxEntry + distInsideBox;
    float marchEnd = min(distToBoxExit, distToGeometry);
    
    if (distToBoxEntry > marchEnd || distInsideBox <= 0.0) {
        discard;
    }

    float totalDistanceToMarch = marchEnd - distToBoxEntry;
    float stepSize = totalDistanceToMarch / float(NB_STEPS);

    vec3 currentPos = viewPos + rayDir * distToBoxEntry;
    
    float transmittance = 1.0;
    vec3 accumulatedColor = vec3(0.0);

    // Scattering (S) = Incoming Light * Phase * Fog Color * Density
    float g = 0.5; // isotropic scattering
    float phase = PhaseFunction(rayDir, uLightDir, g);
    vec3 S = uLightColor * phase * FOG_COLOR * DENSITY;
    float sigmaE = DENSITY; // Extinction coefficient

    for(int i = 0; i < NB_STEPS; i++)
    {
        // vec3 samplePos = currentPos + rayDir * (stepSize * 0.5);

        // Scattering Integration = (S - S * exp(-sigma * d)) / sigma
        vec3 Sint = (S - S * exp(-sigmaE * stepSize)) / sigmaE;
        
        accumulatedColor += transmittance * Sint;
        transmittance *= exp(-sigmaE * stepSize); //Beer's Law
        currentPos += rayDir * stepSize;

        if(transmittance < 0.01) break;
    }

    FragColor = vec4(accumulatedColor, 1.0 - transmittance);
}