#include<pbr.glsl>

uniform sampler2D u_brdfLUT;
uniform samplerCube u_diffuseIBLMap;
uniform samplerCube u_specularIBLMap;
uniform int u_maxSpecularIBLMapMipmapLevel;
uniform float u_IBLContribution;

vec3 fresnelSchlickRoughness(in float cosTheta, in vec3 F0, in float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calcIblLighting(in PbrData pbr, in vec3 F0, in vec3 N, in vec3 V)
{
    float NdotV = max(dot(N, V), 0.0);
    vec3 R = normalize(reflect(-V, N));

    vec3 F = fresnelSchlickRoughness(NdotV, F0, pbr.roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - pbr.metallic;

    vec3 irradiance = texture(u_diffuseIBLMap, N).rgb;
    vec3 diffuse = irradiance * pbr.baseColor;

    vec3 prefilteredColor = textureLod(u_specularIBLMap, R, pbr.roughness * float(u_maxSpecularIBLMapMipmapLevel)).rgb;
    vec2 brdf = texture(u_brdfLUT, vec2(NdotV, pbr.roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + vec3(brdf.y));

    return u_IBLContribution * (kD * diffuse + specular);
}

