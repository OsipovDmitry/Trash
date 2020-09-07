#version 330 core

#include<gammacorrection.glsl>

uniform samplerCube u_specularIBLMap;
uniform int u_numSpecularIBLMapLods;
uniform float u_IBLContribution;
uniform float u_roughness;

in vec3 v_tc;

out vec4 fragColor;

void main(void)
{
    vec3 color = vec3(0.0);
    color += u_IBLContribution * textureLod(u_specularIBLMap, v_tc, u_roughness * float(u_numSpecularIBLMapLods)).rgb;

    color = color / (color + vec3(1.0));
    fragColor = vec4(toSRGB(color), 1.0);
}
