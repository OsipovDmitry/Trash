#include<lights.glsl>
#include<gammacorrection.glsl>

uniform sampler2D u_baseColorMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;

const float PI = 3.14159265359;

struct PbrData
{
    vec3 baseColor;
    float metallic;
    float roughness;
};

PbrData getPbrData(in vec2 texCoord)
{
    PbrData pbr;
    pbr.baseColor = toLinearRGB(texture(u_baseColorMap, texCoord).rgb);
    pbr.metallic = texture(u_metallicMap, texCoord).r;
    pbr.roughness = texture(u_roughnessMap, texCoord).r;
    return pbr;
}

float DistributionGGX(float NdotH, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calcPbrLighting(in PbrData pbr, in vec3 F0, in vec3 lightColor, in vec3 N, in vec3 L, in vec3 V, in float attenuation)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(NdotH, pbr.roughness);
    float G = GeometrySmith(NdotV, NdotL, pbr.roughness);
    vec3 F = fresnelSchlick(HdotV, F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - pbr.metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular = numerator / max(denominator, 1e-3);

    vec3 radiance = lightColor * attenuation;
    return (kD * pbr.baseColor / PI + specular) * radiance * NdotL;
}

