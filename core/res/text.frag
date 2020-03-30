#version 330 core

#include<gammacorrection.glsl>

uniform sampler2D u_fontMap;

uniform vec4 u_color;

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
    vec4 color = texture(u_fontMap, v_texCoord) * u_color;

    color.rgb = color.rgb / (color.rgb + vec3(1.0));
    fragColor = vec4(toSRGB(color.rgb), color.a);
}
