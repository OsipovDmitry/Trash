#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

out vec3 v_normal;
out vec2 v_texCoord;

void main(void)
{
    gl_Position = u_projMatrix * u_viewMatrix * u_modelMatrix * vec4(a_position, 1.0);
    
    v_normal = a_normal;
    v_texCoord = a_texCoord;
}
