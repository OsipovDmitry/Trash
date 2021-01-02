#include<pbr.glsl>

#ifdef IBL
#include<ibl.glsl>
#endif

uniform mat4 u_viewProjMatrixInverse;
uniform vec3 u_viewPosition;
uniform uvec2 u_viewportSize;
uniform uint u_id;

uniform sampler2D u_gBufferMap0;
uniform sampler2D u_gBufferMap1;
uniform sampler2D u_gBufferMap2;

out vec4 fragColor;

void main(void)
{
    vec2 texCoord = (gl_FragCoord.xy) / vec2(u_viewportSize);

    float depth = texture(u_gBufferMap0, texCoord).r;
    vec4 baseColorRoughness = texture(u_gBufferMap1, texCoord);
    vec4 normalMetallic = texture(u_gBufferMap2, texCoord);

    PbrData pbr = PbrData(baseColorRoughness.rgb, normalMetallic.a, baseColorRoughness.a);

    const float dielectricSpecular = 0.04;
    vec3 F0 = mix(vec3(dielectricSpecular), pbr.baseColor.rgb, pbr.metallic);

    vec3 N = 2.0 * normalMetallic.rgb - vec3(1.0);
    float NLen = length(N);

    N /= NLen;

    vec4 pixelPos = u_viewProjMatrixInverse * vec4(2.0 * vec3(texCoord, depth) - vec3(1.0), 1.0);
    pixelPos /= pixelPos.w;

    vec3 V = normalize(u_viewPosition - pixelPos.xyz);

    vec3 color = vec3(0.0);

#ifdef LIGHT
    LightStruct light = u_lights[u_id];

    vec3 toLight = toLightVector(light, pixelPos.xyz);
    vec3 L = normalize(toLight);

    vec4 posLightSpace = LIGHT_MATRIX(light) * pixelPos;

    float shadow = LIGHT_IS_SHADOW_ENABLED(light) ? lightShadow(posLightSpace.xyz / posLightSpace.w, int(u_id)) : 1.0;
    color += calcPbrLighting(pbr, F0, LIGHT_COLOR(light), N, L, V, lightAttenuation(light, toLight)) * shadow;
#else
#ifdef IBL
    color += calcIblLighting(pbr, F0, N, V);
#endif
#endif

    fragColor = vec4(color, 1.0);
    //fragColor = vec4(1,0,0, 1.0);
}
