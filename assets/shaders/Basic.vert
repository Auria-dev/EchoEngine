#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 vTexCoord;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    // vec4 worldPos = uModel * vec4(, 1.0);
    vTexCoord = aTexCoords;
    
    gl_Position = uProjection * uView * uModel * vec4(aPos.xyz,1.0);
}