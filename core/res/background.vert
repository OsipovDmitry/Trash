#version 330 core

layout (location = 0) in vec2 a_position;

uniform mat4 u_backgroundMatrix;

out vec3 v_tc;

void main(void)
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    vec4 texCoord = u_backgroundMatrix * vec4(a_position, 1.0, 1.0);
    v_tc = texCoord.xyz / texCoord.w;
}
