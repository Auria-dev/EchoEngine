#version 330 core

layout(location = 0) out vec4 color;

in vec2 vTexCoord;

uniform sampler2D uAlbedo;
uniform vec4 uColor;

void main()
{
    vec3 textureColor = texture(uAlbedo, vTexCoord).rgb;
    color = vec4(textureColor, 1.0) * uColor;
}