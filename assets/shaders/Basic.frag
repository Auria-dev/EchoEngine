#version 330 core

layout(location = 0) out vec4 color;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uARM;
uniform vec4 uColor;

void main()
{
    vec3 albedo = texture(uAlbedo, fs_in.TexCoords).rgb;
    vec3 normal = texture(uNormal, fs_in.TexCoords).rgb;
    vec3 arm    = texture(uARM, fs_in.TexCoords).rgb;
    color = vec4(albedo, 1.0) * uColor;
}