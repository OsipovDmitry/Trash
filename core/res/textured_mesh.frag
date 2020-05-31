#version 330 core

#define MAX_LIGHTS 8

uniform sampler2D u_baseColorMap;
uniform sampler2D u_opacityMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform samplerCube u_diffuseIBLMap;
uniform samplerCube u_specularIBLMap;
uniform sampler2D u_brdfLUT;
uniform sampler2DArrayShadow u_shadowMaps;

uniform int u_numSpecularIBLMapLods;

#include<gammacorrection.glsl>
#include<lights.glsl>
#include<pbr.glsl>
#include<ibl.glsl>

uniform int u_isMetallicRoughWorkflow;
uniform int u_lightIndices[MAX_LIGHTS];

layout (std140) uniform u_lightsBuffer
{
    LightStruct u_lights[128];
};

in vec3 v_tangent;
in vec3 v_binormal;
in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_toView;
in vec3 v_toLight[MAX_LIGHTS];
in vec4 v_posLightSpace[MAX_LIGHTS];

out vec4 fragColor;

float lightShadow(in LightStruct light, in vec3 posInLightSpace, in int lightIdx)
{
    const int N = 2;

    vec2 shadowMapSize = textureSize(u_shadowMaps, 0).xy;
    float shadow = 0.0;
    int x, y;
    for (x = -N; x <= N; ++x)
        for (y = -N; y <= N; ++y)
            shadow += texture(u_shadowMaps, vec4(posInLightSpace.xy + vec2(x,y) / shadowMapSize, float(lightIdx), posInLightSpace.z));

    return shadow / ((2*N+1) * (2*N+1));
}

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
        int lightIdx = u_lightIndices[i];
        LightStruct light = u_lights[lightIdx];

        float shadow = LIGHT_IS_SHADOW_ENABLED(light) ? lightShadow(light, v_posLightSpace[i].xyz / v_posLightSpace[i].w, lightIdx) : 1.0;
        Lo += calcPbrLighting(pbr, F0, LIGHT_COLOR(light), fragNormal, normalize(v_toLight[i]), toView, lightAttenuation(light, v_toLight[i])) * shadow;
    }

    vec3 color = vec3(0.0);
    color += Lo;
    color += 0.2 * calcIblLighting(pbr, F0, fragNormal, toView);

    color = color / (color + vec3(1.0));
    fragColor = vec4(toSRGB(color), 1.0);
}
