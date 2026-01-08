#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    vec4 viewPos = uView * uModel * vec4(aPos, 1.0);
    vs_out.FragPos = viewPos.xyz;
    vs_out.TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(uView * uModel)));

    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    
    vec3 B_cpu = normalize(normalMatrix * aBitangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B_geo = cross(N, T);
    float handedness = dot(B_geo, B_cpu) < 0.0 ? -1.0 : 1.0;
    vec3 B = B_geo * handedness;

    vs_out.TBN = mat3(T, B, N);

    gl_Position = uProjection * viewPos;
}