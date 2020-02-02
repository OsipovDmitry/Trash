#version 330 core

uniform sampler2D u_baseColorMap;
uniform sampler2D u_opacityMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform samplerCube u_diffuseIBLMap;
uniform samplerCube u_specularIBLMap;
uniform sampler2D u_brdfLUT;

uniform int u_numSpecularIBLMapLods;

#include<:/res/gammacorrection.glsl>
#include<:/res/lights.glsl>
#include<:/res/pbr.glsl>
#include<:/res/ibl.glsl>

#define MAX_LIGHTS 8

uniform int u_isMetallicRoughWorkflow;
uniform int u_lightIndices[MAX_LIGHTS];

layout (std140) uniform u_lightsBuffer
{
    mat4x4 u_lights[128];
};

in vec3 v_tangent;
in vec3 v_binormal;
in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_toView;
in vec3 v_toLight[MAX_LIGHTS];

out vec4 fragColor;

void main(void)
{
    vec3 tangent = normalize(v_tangent);
    vec3 binormal = normalize(v_binormal);
    vec3 normal = normalize(v_normal);
    vec3 fragNormal = normalize(mat3(tangent, binormal, normal) * (texture(u_normalMap, v_texCoord).xyz * 2.0 - vec3(1.0, 1.0, 1.0)));
    vec3 toView = normalize(v_toView);

    PbrData pbr = getPbrData(v_texCoord, u_isMetallicRoughWorkflow > 0);

    vec3 F0 = mix(vec3(dielectricSpecular), pbr.baseColor, pbr.metallic);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        Light light = u_lights[u_lightIndices[i]];
        Lo += calcPbrLighting(pbr, F0, LIGHT_COLOR(light), fragNormal, normalize(v_toLight[i]), toView, lightAttenuation(light, v_toLight[i]));
    }

    vec3 color = vec3(0.0);
    color += Lo;
    color += calcIblLighting(pbr, F0, fragNormal, toView);

    color = color / (color + vec3(1.0));
    fragColor = vec4(toSRGB(color), 1.0);
}
