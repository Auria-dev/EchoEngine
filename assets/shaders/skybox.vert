#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 localPos;

void main()
{
    localPos = aPos;

    mat4 rotView = mat4(mat3(uView)); // remove translation from the uView matrix
    vec4 clipPos = uProjection * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}