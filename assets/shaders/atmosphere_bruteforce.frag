#version 330

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gDepth;
uniform vec3 viewPos; // m^-1
uniform vec3 uLightDir;
uniform mat4 uInvViewProj;

const float PI        = 3.14159265359;
const float RGround   = 6360.0;
const float RTop      = 6460.0;
const float RhoAlbedo = 0.3;

// convert to KM
const vec3  RayleighScattering = vec3(5.802, 13.558, 33.1) * 0.001;
const float RayleighAbsorption = 0.0;

const float MieScattering      = 3.996 * 0.001; 
const float MieAbsorption      = 4.40  * 0.001;

const float OzoneScattering    = 0.0;
const vec3  OzoneAbsorption    = vec3(0.650, 1.881, 0.085) * 0.001;

const vec3 RayleighExtinction =      RayleighScattering + vec3(RayleighAbsorption);
const vec3 MieExtinction      = vec3(MieScattering)     + vec3(MieAbsorption);
const vec3 OzoneExtinction    = vec3(OzoneScattering)   +      OzoneAbsorption;


struct Light
{
    vec3 l; // direction
    float E; // illuminance
};


vec3 GetWorldPos(float depth, vec2 uv) {
    float z = depth * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 view = uInvViewProj * clip;
    return view.xyz / view.w;
}

vec2 RaySphereIntersection(vec3 rayOrigin, vec3 rayDir, float radius) {
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(rayOrigin, rayDir);
    float c = dot(rayOrigin, rayOrigin) - (radius * radius);
    float d = (b * b) - 4.0 * a * c;
    
    if (d < 0.0) return vec2(-1.0); // No intersection
    
    float t1 = (-b - sqrt(d)) / (2.0 * a);
    float t2 = (-b + sqrt(d)) / (2.0 * a);
    return vec2(t1, t2);
}

// theta: angle between incident and outgoing scattering directions
float RayLeighPhase(float cosTheta) // p_r
{
    return 3.0 * (1 + (cosTheta*cosTheta)) / (16.0 * PI);
}

// g: asymmetry parameter in ]-1,1[
float MiePhase(float cosTheta, float g) // p_m
{
    float num   = (1.0 - g*g) * (1.0 + (cosTheta * cosTheta));
    float denom = (2.0 + g*g) * pow(1.0 + g*g - 2.0 * g * cosTheta, 1.5);    
    return (3.0 / (8.0 * PI)) * (num / denom);
}

const float IsotropicPhase = 1.0 / (4.0 * PI); // p_u

float RayleighAltitudeDensityDistribution(float h)
{
    return exp(-h/8.0); // 8km
}

float MieAltitudeDensityDistribution(float h)
{
    return exp(-h/1.2); // 1.2km
}

float OzongAltitureDensityDistribution(float h)
{
    return max(0.0, 1.0-(abs(h-25.0)/15.0));
}

vec3 Transmittance(vec3 x_a, vec3 x_b)
{
    float dist = length(x_b - x_a);
    vec3  dir  = (x_b - x_a) / dist;

    const int STEPS = 40;
    float stepSize = dist / float(STEPS);
    
    vec3 accumulatedOpticalDepth = vec3(0.0);
    vec3 currentPos = x_a + dir * (stepSize * 0.5);

    for(int i = 0; i < STEPS; ++i)
    {
        float h = max(0.0, length(currentPos) - RGround);

        if (h > (RTop - RGround))
        {
             currentPos += dir * stepSize;
             continue; 
        }

        float d_rayleigh = RayleighAltitudeDensityDistribution(h);
        float d_mie      = MieAltitudeDensityDistribution(h);
        float d_ozone    = OzongAltitureDensityDistribution(h);

        vec3 sigma_t = (RayleighExtinction * d_rayleigh) +
                       (MieExtinction      * d_mie)      +
                       (OzoneExtinction    * d_ozone);

        accumulatedOpticalDepth += sigma_t * stepSize;
        currentPos += dir * stepSize;
    }

    return exp(-accumulatedOpticalDepth);
}

void main()
{
    float depthVal = texture(gDepth, TexCoords).r;
    vec3 worldPos = GetWorldPos(depthVal, TexCoords);
    
    vec3 camPos = viewPos * 0.001; // to km
    vec3 rayDir = normalize(worldPos - viewPos);
    
    // Check where the camera ray hits the atmosphere sphere
    camPos.y += RGround;
    vec2 t_atmo = RaySphereIntersection(camPos, rayDir, RTop);
    if (t_atmo.y < 0.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Looking into space, no atmosphere
        return;
    }

    float tStart = max(0.0, t_atmo.x); 
    
    // Stop at the ground or the back of the atmosphere, whichever is closer
    vec2 t_ground = RaySphereIntersection(camPos, rayDir, RGround);
    float tEnd = t_atmo.y;
    if (t_ground.x > 0.0) {
        tEnd = min(tEnd, t_ground.x);
    }
    
    const int STEPS = 32;
    float dt = (tEnd - tStart) / float(STEPS);
    vec3 currentPos = camPos + rayDir * (tStart + dt * 0.5); // Mid-point integration
    
    vec3 L = vec3(0.0);
    vec3 T_view = vec3(1.0);  // Transmittance from Camera to currentPos
    
    // Sun Setup (Assume Directional Light)
    vec3 sunDir = normalize(uLightDir); 
    float sunE = 20.0; // Sun Intensity

    // Phase Functions (Constant per-pixel for directional light)
    float mu = dot(rayDir, sunDir);
    float phaseR = RayLeighPhase(mu); 
    float phaseM = MiePhase(mu, 0.8); // g = 0.8 is standard for Mie

    for (int i = 0; i < STEPS; ++i) {
        float h = length(currentPos) - RGround;
        
        // Get Densities at this altitude
        float d_r = RayleighAltitudeDensityDistribution(h);
        float d_m = MieAltitudeDensityDistribution(h);
        float d_o = OzongAltitureDensityDistribution(h);
        
        // Calculate Local Scattering/Extinction coefficients
        // Note: Ozone absorbs but does not scatter!
        vec3 sigma_s_r = RayleighScattering * d_r;
        vec3 sigma_s_m = vec3(MieScattering) * d_m;
        
        vec3 sigma_t = (RayleighExtinction * d_r) +
                       (MieExtinction      * d_m) +
                       (OzoneExtinction    * d_o);
        
        // Calculate Transmittance from Sun to currentPos
        // This is the "Shadow" S(x) term (without visibility checks for now)
        // Warning: This calling Transmittance() inside the loop makes it O(N^2)!
        vec2 t_sun_boundary = RaySphereIntersection(currentPos, sunDir, RTop);
        vec3 sunPos_boundary = currentPos + sunDir * t_sun_boundary.y;
        vec3 T_sun = Transmittance(currentPos, sunPos_boundary);
        
        // Calculate Scattering
        // Scattering = (Rayleigh_Scat * PhaseR + Mie_Scat * PhaseM) * T_sun * Sun_Intensity
        vec3 S = (sigma_s_r * phaseR + sigma_s_m * phaseM) * T_sun * sunE;
        
        // Accumulate Radiance
        // L += S * T_view * dt
        L += S * T_view * dt;
        
        // Update View Transmittance for the NEXT step
        // T_view *= exp(-sigma_t * dt)
        T_view *= exp(-sigma_t * dt);
        
        currentPos += rayDir * dt;
    }
    
    // If the ray hit the ground, we add the light reflecting off the surface
    if (t_ground.x > 0.0) {
        // Simple Lambertian ground
        // We use the REMAINING T_view because that's what made it through the atmosphere
        vec3 groundAlbedo = vec3(RhoAlbedo); 
        vec3 groundNormal = normalize(camPos + rayDir * tEnd); // Normal is vector from center
        float NdotL = max(0.0, dot(groundNormal, sunDir));
        
        // Note: We need T_sun at the ground surface too!
        vec3 sunPos_ground = (camPos + rayDir * tEnd) + sunDir * RaySphereIntersection(camPos + rayDir * tEnd, sunDir, RTop).y;
        vec3 T_sun_ground = Transmittance(camPos + rayDir * tEnd, sunPos_ground);
        
        vec3 L0 = groundAlbedo * (sunE * T_sun_ground * NdotL) / PI;
        L += L0 * T_view;
    }

    L = L / (L + vec3(1.0));
    L = pow(L, vec3(1.0/2.2));
    FragColor = vec4(L, 1.0);

    if (dot(rayDir, uLightDir) > .99995) FragColor = vec4(1.0,1.0,1.0,1.0);
}