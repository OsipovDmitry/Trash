
#include<:/res/pbr.glsl>
#include<:/res/gammacorrection.glsl>

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calcIblLighting(in PbrData pbr, in vec3 F0, in vec3 n, in vec3 v)
{
    float NdotV = clamp(dot(n, v), 0.0, 1.0);
    vec3 r = normalize(reflect(-v, n));

    vec3 F = fresnelSchlickRoughness(NdotV, F0, pbr.roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - pbr.metallic;

    vec3 irradiance = texture(u_diffuseIBLMap, n).rgb;
    vec3 diffuse = irradiance * pbr.baseColor;

    vec3 prefilteredColor = textureLod(u_specularIBLMap, r, pbr.roughness * float(u_numSpecularIBLMapLods)).rgb;
    vec2 brdf = texture(u_brdfLUT, vec2(NdotV, pbr.roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + vec3(brdf.y));

    return kD * diffuse + specular;
}

