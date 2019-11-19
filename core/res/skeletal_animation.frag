#version 330 core

in vec3 v_position;

out vec4 fragColor;

void main(void)
{
    fragColor = vec4(v_position, 1.0);
}
