#version 330 core
out vec3 FragColor;
in vec2 TexCoords;

uniform sampler2D uTransmittanceLUT;

const float PI      = 3.14159265359;
const float RGround = 6360.0;
const float RTop    = 6460.0;

const vec3 RayleighScattering = vec3(5.802, 13.558, 33.1) * 0.001;
const vec3 MieScattering      = vec3(3.996, 3.996, 3.996) * 0.001;
const vec3 RayleighExtinction = RayleighScattering;
const vec3 MieExtinction      = vec3(3.996 + 4.40) * 0.001;

const float IsotropicPhase = 1.0 / (4.0 * PI);

float RayleighDensity(float h) { return exp(-h/8.0); }
float MieDensity(float h)      { return exp(-h/1.2); }

vec3 GetTransmittance(float altitude, float cosTheta)
{
    float u = clamp((cosTheta + 1.0) / 2.0, 0.0, 1.0);
    float v = clamp(altitude / (RTop - RGround), 0.0, 1.0);
    return texture(uTransmittanceLUT, vec2(u, v)).rgb;
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

void main()
{
    float u = TexCoords.x;
    float v = TexCoords.y;
    
    float cosSunZenith = 2.0 * u - 1.0;
    float h = RGround + (RTop - RGround) * v;
    vec3 sunDir = vec3(sqrt(max(0.0, 1.0 - cosSunZenith*cosSunZenith)), cosSunZenith, 0.0);
    vec3 viewPos = vec3(0.0, h, 0.0);

    const int SAMPLES_SQRT = 8;
    vec3 L2ndOrder = vec3(0.0);
    vec3 F_ms = vec3(0.0);

    for (int i = 0; i < SAMPLES_SQRT; ++i)
    {
        for (int j = 0; j < SAMPLES_SQRT; ++j)
        {
            float theta = PI * (float(i) + 0.5) / float(SAMPLES_SQRT);
            float phi = 2.0 * PI * (float(j) + 0.5) / float(SAMPLES_SQRT);
            vec3 rayDir = vec3(sin(theta)*cos(phi), cos(theta), sin(theta)*sin(phi));
            
            vec2 t_atmo = RaySphereIntersection(viewPos, rayDir, RTop);
            vec2 t_ground = RaySphereIntersection(viewPos, rayDir, RGround);
            float tEnd = t_atmo.y;
            if (t_ground.x > 0.0) tEnd = min(tEnd, t_ground.x);
            
            const int STEPS = 20;
            float dt = tEnd / float(STEPS);
            vec3 currentPos = viewPos + rayDir * (dt * 0.5);
            
            vec3 L_scat = vec3(0.0);
            vec3 T_ray = vec3(1.0);

            for (int k = 0; k < STEPS; ++k) {
                float h_sample = length(currentPos) - RGround;
                float d_r = RayleighDensity(h_sample);
                float d_m = MieDensity(h_sample);
                
                vec3 sigma_s = RayleighScattering * d_r + MieScattering * d_m;
                vec3 sigma_t = RayleighExtinction * d_r + MieExtinction * d_m;
                
                float cosThetaSun = dot(normalize(currentPos), sunDir);
                vec3 T_sun = GetTransmittance(h_sample, cosThetaSun);
                
                vec3 S = sigma_s * T_sun * IsotropicPhase; 
                L_scat += S * T_ray * dt;
                
                T_ray *= exp(-sigma_t * dt);
                currentPos += rayDir * dt;
            }
            
            float weight = (PI / float(SAMPLES_SQRT)) * (2.0 * PI / float(SAMPLES_SQRT)) * sin(theta);
            
            L2ndOrder += L_scat * weight;
            F_ms += T_ray * weight;
        }
    }
    
    vec3 Psi_ms = L2ndOrder / (1.0 - F_ms);
    
    FragColor = Psi_ms;
}