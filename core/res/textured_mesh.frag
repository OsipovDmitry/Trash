#version 330 core

uniform sampler2D u_diffuseMap;
uniform sampler2D u_normalMap;

in vec3 v_normal;
in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(u_diffuseMap, v_texCoord);
}
