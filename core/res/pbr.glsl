#include<gammacorrection.glsl>

struct PbrData
{
    vec3 baseColor;
    float metallic;
    float roughness;
};

const float dielectricSpecular = 0.04;
const float PI = 3.14159265359;

float getPerceivedBrightness(in vec3 color)
{
    return sqrt(0.299 * color.r * color.r + 0.587 * color.g * color.g + 0.114 * color.b * color.b);
}

float solveMetallic(in float diffuse, in float specular, in float oneMinusSpecularStrength)
{
    if (specular < dielectricSpecular)
        return 0.0;

    float a = dielectricSpecular;
    float b = diffuse * oneMinusSpecularStrength / (1.0 - dielectricSpecular) + specular - 2.0 * dielectricSpecular;
    float c = dielectricSpecular - specular;
    float D = max(0.0, b*b - 4*a*c);
    return clamp((-b+sqrt(D)) / (2.0*a), 0.0, 1.0);
}

PbrData getPbrData(in vec2 texCoord, in bool isMetaalicRoughWorkflow)
{
    PbrData pbr;
    if (isMetaalicRoughWorkflow)
    {
        pbr.baseColor = toLinearRGB(texture(u_baseColorMap, texCoord).rgb);
        pbr.metallic = texture(u_metallicMap, texCoord).r;
        pbr.roughness = texture(u_roughnessMap, texCoord).r;
    }
    else
    {
        vec3 diffuse = toLinearRGB(texture(u_baseColorMap, texCoord).rgb);
        vec3 specular = toLinearRGB(texture(u_metallicMap, texCoord).rgb);
        float glossiness = texture(u_roughnessMap, texCoord).r;

        float oneMinusSpecularStrength = 1.0 - max(specular.r, max(specular.g, specular.b));
        float metallic = solveMetallic(getPerceivedBrightness(diffuse), getPerceivedBrightness(specular), oneMinusSpecularStrength);

        vec3 baseColorFromDiffuse = diffuse * (oneMinusSpecularStrength / (1.0 - dielectricSpecular) / max(1.0 - metallic, 1e-3));
        vec3 baseColorFromSpecular = (specular - vec3(dielectricSpecular) * (1.0 - metallic)) * (1.0 / max(metallic, 1e-3));

        pbr.baseColor = mix(baseColorFromDiffuse, baseColorFromSpecular, metallic * metallic);
        pbr.metallic = metallic;
        pbr.roughness = 1.0 - glossiness;
    }

    return pbr;
}


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
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
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calcPbrLighting(in PbrData pbr, in vec3 F0, in vec3 lightColor, in vec3 n, in vec3 l, in vec3 v, in float attenuation)
{
    vec3 h = normalize(l + v);
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(n, h, pbr.roughness);
    float G = GeometrySmith(n, v, l, pbr.roughness);
    vec3 F = fresnelSchlick(max(dot(h, v), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - pbr.metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0);
    vec3 specular = numerator / max(denominator, 1e-3);

    float NdotL = max(dot(n, l), 0.0);
    return (kD * pbr.baseColor / PI + specular) * radiance * NdotL;
}

