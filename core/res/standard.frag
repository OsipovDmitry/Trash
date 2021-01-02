#ifdef HAS_LIGHTING
#include<lights.glsl>
#include<pbr.glsl>
#include<ibl.glsl>
#endif

#ifdef HAS_NORMALMAPPING
uniform sampler2D u_normalMap;
#endif

#ifdef HAS_OPACITYMAPPING
uniform sampler2D u_opacityMap;
#endif

#ifdef HAS_LIGHTING
uniform int u_lightIndices[MAX_LIGHTS];
#endif

uniform vec4 u_color;

#ifdef HAS_LIGHTING
in vec3 v_normal;
in vec3 v_toLight[MAX_LIGHTS];
in vec4 v_posLightSpace[MAX_LIGHTS];
#ifdef HAS_NORMALMAPPING
in vec3 v_tangent;
in vec3 v_binormal;
#endif
#endif

in vec2 v_texCoord;
in vec3 v_toView;

out vec4 fragColor;

void main(void)
{
    vec3 color = vec3(0.0);

#ifdef HAS_LIGHTING
    vec3 N = normalize(v_normal);

#ifdef HAS_NORMALMAPPING
    vec3 tangent = normalize(v_tangent);
    vec3 binormal = normalize(v_binormal);
    N = normalize(mat3(tangent, binormal, N) * (texture(u_normalMap, v_texCoord).xyz * 2.0 - vec3(1.0, 1.0, 1.0)));
#endif

    PbrData pbr = getPbrData(v_texCoord);
    pbr.baseColor *= u_color.rgb;

    vec3 V = normalize(v_toView);

    const float dielectricSpecular = 0.04;
    vec3 F0 = mix(vec3(dielectricSpecular), pbr.baseColor.rgb, pbr.metallic);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        int lightIdx = u_lightIndices[i];
        LightStruct light = u_lights[lightIdx];

        vec3 L = normalize(v_toLight[i]);

        float shadow = LIGHT_IS_SHADOW_ENABLED(light) ? lightShadow(v_posLightSpace[i].xyz / v_posLightSpace[i].w, lightIdx) : 1.0;
        Lo += calcPbrLighting(pbr, F0, LIGHT_COLOR(light), N, L, V, lightAttenuation(light, v_toLight[i])) * shadow;
    }

    color += Lo;
    color += calcIblLighting(pbr, F0, N, V);
#else
    color = u_color.rgb;
#endif

    float opacity = u_color.a;
#ifdef HAS_OPACITYMAPPING
    opacity *= texture(u_opacityMap, v_texCoord).r;
#endif

    fragColor = vec4(color, opacity);
}
