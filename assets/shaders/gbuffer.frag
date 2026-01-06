#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gARM;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uARM;

void main() {
    gPosition = vec4(fs_in.FragPos, 1.0);

    vec3 tNormal = texture(uNormal, fs_in.TexCoords).rgb;
    tNormal = normalize(tNormal * 2.0 - 1.0);
    vec3 viewNormal = normalize(fs_in.TBN * tNormal);
    
    gNormal = vec4(viewNormal, 1.0);

    vec3 tAlbedo = texture(uAlbedo, fs_in.TexCoords).rgb;
    vec3 tARM    = texture(uARM, fs_in.TexCoords).rgb;

    gAlbedo = vec4(tAlbedo, 1.0);
    gARM    = vec4(tARM, 1.0);
}