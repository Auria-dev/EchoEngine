#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uARM;

uniform float uOpacity;

void main()
{
    vec4 albedo = texture(uAlbedo, fs_in.TexCoords);
    float alpha = albedo.a * uOpacity;
    if(alpha < 0.01) discard;
    FragColor = vec4(albedo.rgb, alpha);
}