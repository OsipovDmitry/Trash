#version 330 core

in vec3 a_position;
in vec3 a_color;

out vec3 v_color;

void main(void)
{
    gl_Position = vec4(a_position, 1.0);
    v_color = a_color;
}
