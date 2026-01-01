#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gARM;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uARM;

void main() {
    gPosition = fs_in.FragPos;

    vec3 tNormal = texture(uNormal, fs_in.TexCoords).rgb;
    tNormal = normalize(tNormal * 2.0 - 1.0);

    vec3 viewNormal = normalize(fs_in.TBN * tNormal);
    gNormal.rgb = viewNormal;

    vec3 tAlbedo = texture(uAlbedo, fs_in.TexCoords).rgb;
    vec3 tARM    = texture(uARM, fs_in.TexCoords).rgb;

    gAlbedo.rgb = tAlbedo;
    gARM.rgb = tARM;
}