#version 330 core
out vec3 FragColor;
in vec2 TexCoords;

const float PI = 3.14159265359;
const float RGround = 6360.0;
const float RTop = 6460.0;

const vec3  RayleighScattering = vec3(5.802, 13.558, 33.1) * 0.001;
const float RayleighAbsorption = 0.0;
const float MieScattering      = 3.996 * 0.001; 
const float MieAbsorption      = 4.40  * 0.001;
const float OzoneScattering    = 0.0;
const vec3  OzoneAbsorption    = vec3(0.650, 1.881, 0.085) * 0.001;

const vec3 RayleighExtinction = RayleighScattering + vec3(RayleighAbsorption);
const vec3 MieExtinction      = vec3(MieScattering) + vec3(MieAbsorption);
const vec3 OzoneExtinction    = vec3(OzoneScattering) + OzoneAbsorption;

float RayleighAltitudeDensityDistribution(float h) { return exp(-h/8.0); }
float MieAltitudeDensityDistribution(float h)      { return exp(-h/1.2); }
float OzoneAltitudeDensityDistribution(float h)    { return max(0.0, 1.0-(abs(h-25.0)/15.0)); }

vec3 IntegrateOpticalDepth(vec3 pos, vec3 dir)
{
    float a = dot(dir, dir);
    float b = 2.0 * dot(pos, dir);
    float c = dot(pos, pos) - (RGround * RGround);
    float d = b*b - 4.0*a*c;

    float distToGround = -1.0;
    if (d >= 0.0)
    {
        float t1 = (-b - sqrt(d)) / (2.0 * a);
        float t2 = (-b + sqrt(d)) / (2.0 * a);
        if (t1 > 0.0) distToGround = t1;
        else if (t2 > 0.0) distToGround = t2;
    }

    c = dot(pos, pos) - (RTop * RTop);
    d = b*b - 4.0*a*c;
    
    float tTop = (-b + sqrt(d)) / (2.0 * a); 
    float dist = tTop;
    if (distToGround > 0.0)
    {
        return vec3(1e9); 
    }
    
    const int STEPS = 1024;
    float stepSize = dist / float(STEPS);
    vec3 opticalDepth = vec3(0.0);
    vec3 currentPos = pos + dir * (stepSize * 0.5);

    for(int i = 0; i < STEPS; ++i)
    {
        float h = length(currentPos) - RGround;
        
        if (h < 0.0)
        {
            currentPos += dir * stepSize;
            continue;
        }

        float d_r = RayleighAltitudeDensityDistribution(h);
        float d_m = MieAltitudeDensityDistribution(h);
        float d_o = OzoneAltitudeDensityDistribution(h);

        vec3 sigma_t = (RayleighExtinction * d_r) + (MieExtinction * d_m) + (OzoneExtinction * d_o);
        opticalDepth += sigma_t * stepSize;
        currentPos += dir * stepSize;
    }
    return opticalDepth;
}

void main()
{
    float u = TexCoords.x;
    float v = TexCoords.y;
    float h = RGround + (RTop - RGround) * v;
    vec3 pos = vec3(0.0, h, 0.0);
    float cosTheta = 2.0 * u - 1.0;
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta*cosTheta));
    vec3 dir = vec3(sinTheta, cosTheta, 0.0);
    vec3 opticalDepth = IntegrateOpticalDepth(pos, dir);
    FragColor = exp(-opticalDepth);
}