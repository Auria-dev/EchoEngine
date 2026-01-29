#version 330

out vec4 FragColor;
in vec2 TexCoords;
in vec3 vLocalPos;

uniform sampler2D gDepth;
uniform sampler2D gScene;
uniform sampler2D uTransmittanceLUT;
uniform sampler2D uMultiScatteringLUT;
uniform sampler2D uShadowMap;
uniform vec3 viewPos; // m^-1
uniform vec3 uLightDir;
uniform mat4 uInvViewProj;
uniform mat4 uLightProj;

uniform bool uIsIBLPass;

uniform float exposure;

const float PI        = 3.14159265359;
const float RGround   = 6360.0;
const float RTop      = 6460.0;
const float RhoAlbedo = 0.3;

const vec3  RayleighScattering = vec3(5.802, 13.558, 33.1) * 0.001;
const float RayleighAbsorption = 0.0;

const float MieScattering     = 3.996 * 0.001; 
const float MieAbsorption     = 4.40  * 0.001;

const float OzoneScattering   = 0.0;
const vec3  OzoneAbsorption   = vec3(0.650, 1.881, 0.085) * 0.001;

const vec3 RayleighExtinction = RayleighScattering    + vec3(RayleighAbsorption);
const vec3 MieExtinction      = vec3(MieScattering)   + vec3(MieAbsorption);
const vec3 OzoneExtinction    = vec3(OzoneScattering) +      OzoneAbsorption;

float IGN(vec2 uv)
{
    return mod(52.9829189 * mod(0.06711056*uv.x + 0.00583715*uv.y, 1.0), 1.0);
}

const float bayer4x4[16] = float[](
    0.0/16.0, 12.0/16.0, 3.0/16.0, 15.0/16.0,
    8.0/16.0, 4.0/16.0, 11.0/16.0, 7.0/16.0,
    2.0/16.0, 14.0/16.0, 1.0/16.0, 13.0/16.0,
    10.0/16.0, 6.0/16.0, 9.0/16.0, 5.0/16.0
);

float GetBayer4x4(vec2 uv) {
    int x = int(mod(uv.x, 4.0));
    int y = int(mod(uv.y, 4.0));
    return bayer4x4[y * 4 + x];
}

vec3 GetWorldPos(float depth, vec2 uv) {
    float z = depth * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 view = uInvViewProj * clip;
    return view.xyz / view.w;
}

vec2 RaySphereIntersection(vec3 rayOrigin, vec3 rayDir, float radius)
{
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(rayOrigin, rayDir);
    float c = dot(rayOrigin, rayOrigin) - (radius * radius);
    float d = (b * b) - 4.0 * a * c;
    if (d < 0.0) return vec2(-1.0);
    float t1 = (-b - sqrt(d)) / (2.0 * a);
    float t2 = (-b + sqrt(d)) / (2.0 * a);
    return vec2(t1, t2);
}

vec3 GetTransmittanceFromLUT(vec3 pos, vec3 sunDir)
{
    float altitude = length(pos) - RGround;
    float v = clamp(altitude / (RTop - RGround), 0.0, 1.0);
    vec3 up = normalize(pos); 
    float cosTheta = dot(up, normalize(sunDir));
    float u = clamp((cosTheta + 1.0) / 2.0, 0.0, 1.0);
    return texture(uTransmittanceLUT, vec2(u, v)).rgb;
}

vec3 GetMultiScattering(vec3 pos, float cosSunZenith)
{
    float h = length(pos) - RGround;
    float u = clamp((cosSunZenith + 1.0) / 2.0, 0.0, 1.0);
    float v = clamp(h / (RTop - RGround), 0.0, 1.0);
    return texture(uMultiScatteringLUT, vec2(u, v)).rgb;
}

float CalculateShadow(vec3 worldPos)
{
    vec4 fragPosLightSpace = uLightProj * vec4(worldPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0 || projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
    {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

float RayLeighPhase(float cosTheta) { return 3.0 * (1 + (cosTheta*cosTheta)) / (16.0 * PI); } // p_r
float MiePhase(float cosTheta, float g) // p_m
{
    float num   = (1.0 - g*g) * (1.0 + (cosTheta * cosTheta));
    float denom = (2.0 + g*g) * pow(1.0 + g*g - 2.0 * g * cosTheta, 1.5);    
    return (3.0 / (8.0 * PI)) * (num / denom);
}

const float IsotropicPhase = 1.0 / (4.0 * PI); // p_u
float RayleighAltitudeDensityDistribution(float h) { return exp(-h/8.0); }
float MieAltitudeDensityDistribution(float h) { return exp(-h/1.2); }
float OzongAltitureDensityDistribution(float h) { return max(0.0, 1.0-(abs(h-25.0)/15.0)); }

void main()
{
    float depthVal = texture(gDepth, TexCoords).r;
    vec3 worldPos = GetWorldPos(depthVal, TexCoords);
    vec3 rayDir = normalize(worldPos - viewPos);

    float ditherValue = IGN(gl_FragCoord.xy);

    if (uIsIBLPass) {
        depthVal = 1.0;
        vec4 clip = vec4(TexCoords * 2.0 - 1.0, 1.0, 1.0);
        vec4 view = uInvViewProj * clip;
        rayDir = normalize(view.xyz / view.w - viewPos);
    }
    
    vec3 camPosKM = viewPos * 0.001;
    camPosKM.y += RGround + 1.6;
    float sceneDistKM = length(worldPos - viewPos) * 0.001;
    bool hitsGeometry = !uIsIBLPass && (depthVal < 1.0);

    vec2 t_atmo = RaySphereIntersection(camPosKM, rayDir, RTop);
    if (t_atmo.y < 0.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float tStart = max(0.0, t_atmo.x);
    float tEnd = t_atmo.y;
    
    if (hitsGeometry) {
        tEnd = min(tEnd, sceneDistKM);
    }

    vec2 t_ground = RaySphereIntersection(camPosKM, rayDir, RGround);
    if (t_ground.x > 0.0) tEnd = min(tEnd, t_ground.x);

    int STEPS = hitsGeometry ? 16 : 32;
    float dt = (tEnd - tStart) / float(STEPS);
    vec3 currentPos = camPosKM + rayDir * (tStart + dt * 0.5);
    float tCurrent = tStart + (dt*ditherValue);

    vec3 L = vec3(0.0);
    vec3 T_view = vec3(1.0);
    vec3 sunDir = normalize(uLightDir);
    float sunE = 20.0;

    float mu     = dot(rayDir, sunDir);
    float phaseR = RayLeighPhase(mu);
    float phaseM = MiePhase(mu, 0.8);
    float tPrev = tStart;

    for (int i = 0; i < STEPS; ++i)
    {
        float p = (float(i) + ditherValue) / float(STEPS);
        float tCurrent = tStart + (tEnd - tStart) * (p * p);
        float dt = tCurrent - tPrev;
        dt = max(dt, 0.00001);
        vec3 currentPos = camPosKM + rayDir * (tPrev + dt * 0.5);

        float h = length(currentPos) - RGround;

        float d_r = RayleighAltitudeDensityDistribution(h);
        float d_m = MieAltitudeDensityDistribution(h);
        float d_o = OzongAltitureDensityDistribution(h);

        vec3 sigma_s_r = RayleighScattering * d_r;
        vec3 sigma_s_m = vec3(MieScattering) * d_m;
        vec3 sigma_t   = (RayleighExtinction * d_r) +
                         (MieExtinction      * d_m) +
                         (OzoneExtinction    * d_o);

        vec3 T_sun = GetTransmittanceFromLUT(currentPos, sunDir);

        float distFromCamMeters = (length(currentPos - camPosKM)) * 1000.0;
        vec3 sampleWorldPos = viewPos + (rayDir * distFromCamMeters);
        
        float shadow = CalculateShadow(sampleWorldPos);
        float lightVisibility = 1.0 - shadow;
        if (uIsIBLPass) lightVisibility = 1.0;

        vec3 singleScattering = (sigma_s_r * phaseR + sigma_s_m * phaseM) * T_sun * lightVisibility;
        
        vec3 ms = GetMultiScattering(currentPos, dot(normalize(currentPos), sunDir));
        vec3 multiScattering = ms * (sigma_s_r + sigma_s_m);

        vec3 S = (singleScattering + multiScattering) * sunE;

        L += S * T_view * dt;
        T_view *= exp(-sigma_t * dt);
        
        tPrev = tCurrent;
    }
    
    vec3 sceneColor = vec3(0.0);
    if (!uIsIBLPass) {
        sceneColor = texture(gScene, TexCoords).rgb;
    }

    vec3 finalColor = (sceneColor * T_view) + L;

    if (!uIsIBLPass) {
        finalColor = finalColor * exposure;
        const float a = 2.51;
        const float b = 0.03;
        const float c = 2.43;
        const float d = 0.59;
        const float e = 0.14;
        finalColor = clamp((finalColor * (a * finalColor + b)) / (finalColor * (c * finalColor + d) + e), 0.0, 1.0);
        finalColor = pow(finalColor, vec3(1.0 / 2.2));
    }
    
    FragColor = vec4(finalColor, T_view.g);

    float sunDot = dot(rayDir, uLightDir);
    float sunAngularRadius = 0.9999;
    float sunIntensity = smoothstep(sunAngularRadius, sunAngularRadius + 0.0001, sunDot);
    
    if (sunIntensity > 0.0 && !hitsGeometry && t_ground.x < 0.0 && !uIsIBLPass)
    {
        FragColor.rgb = FragColor.rgb + (vec3(1.0) * sunIntensity * T_view);
    }
}
