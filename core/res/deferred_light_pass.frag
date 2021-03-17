#include<lights.glsl>
#include<pbr.glsl>

#ifdef IBL
#include<ibl.glsl>
#endif

uniform mat4 u_viewProjMatrixInverse;
uniform mat4 u_viewMatrixInverse;
uniform vec3 u_viewPosition;
uniform uvec2 u_viewportSize;
uniform uint u_id;
uniform float u_ssaoContribution;

uniform sampler2D u_gBufferMap0;
uniform sampler2D u_gBufferMap1;
uniform sampler2D u_gBufferMap2;
uniform sampler2D u_ssaoMap;

out vec4 fragColor;

void main(void)
{
    vec2 normalizedTexCoord = gl_FragCoord.xy / vec2(u_viewportSize - 1u);

    float depth = texelFetch(u_gBufferMap0, ivec2(normalizedTexCoord * (textureSize(u_gBufferMap0, 0) - 1)), 0).r;
    vec4 baseColorRoughness = texelFetch(u_gBufferMap1, ivec2(normalizedTexCoord * (textureSize(u_gBufferMap1, 0) - 1)), 0);
    vec4 normalXYIntensityMetallic = texelFetch(u_gBufferMap2, ivec2(normalizedTexCoord * (textureSize(u_gBufferMap2, 0) - 1)), 0);

    vec2 Nxy = 2.0 * normalXYIntensityMetallic.rg - vec2(1.0);
    float Nz = sqrt(1.0 - min(Nxy.x*Nxy.x + Nxy.y*Nxy.y, 1.0));
    vec3 N = vec3(u_viewMatrixInverse * vec4(Nxy, Nz, 0.0));

    float intensity = normalXYIntensityMetallic.b * 1023.0 + 1.0;

    vec4 pixelPos = u_viewProjMatrixInverse * vec4(2.0 * vec3(normalizedTexCoord, depth) - vec3(1.0), 1.0);
    pixelPos /= pixelPos.w;

    vec3 V = normalize(u_viewPosition - pixelPos.xyz);

    PbrData pbr = PbrData(baseColorRoughness.rgb * intensity, normalXYIntensityMetallic.a, baseColorRoughness.a);

    const float dielectricSpecular = 0.04;
    vec3 F0 = mix(vec3(dielectricSpecular), pbr.baseColor.rgb, pbr.metallic);

    vec3 color = vec3(0.0);

#ifdef LIGHT
    vec3 toLight = toLightVector(u_id, pixelPos.xyz);
    vec3 L = normalize(toLight);

    vec4 posInLightSpace = LIGHT_MATRIX(u_lights[u_id]) * pixelPos;
    posInLightSpace /= posInLightSpace.w;

    color += calcPbrLighting(pbr, F0, LIGHT_COLOR(u_lights[u_id]), N, L, V, lightAttenuation(u_id, toLight)) * lightShadow(u_id, posInLightSpace.xyz);
#elif defined(IBL)
    color += calcIblLighting(pbr, F0, N, V);
#endif

    float ssao = u_ssaoContribution * texelFetch(u_ssaoMap, ivec2(normalizedTexCoord * (textureSize(u_ssaoMap, 0) - 1)), 0).r;
    color *= (1.0 - ssao);

    fragColor = vec4(color, 1.0);
}
