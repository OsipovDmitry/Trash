#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 2) in vec2 a_texCoord;

uniform mat4 u_modelViewProjMatrix;

out vec2 v_texCoord;

void main(void)
{
    gl_Position = u_modelViewProjMatrix * vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
}
