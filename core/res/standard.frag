#include<gammacorrection.glsl>

#ifdef HAS_LIGHTING
#include<lights.glsl>
#include<pbr.glsl>
#include<ibl.glsl>
#endif

uniform vec4 u_color;
uniform vec2 u_metallicRoughness;

#if defined(HAS_TEXCOORDS) && defined(HAS_BASECOLORMAPPING)
uniform sampler2D u_baseColorMap;
#endif

#if defined(HAS_TEXCOORDS) && defined(HAS_OPACITYMAPPING)
uniform sampler2D u_opacityMap;
#endif

#if defined(HAS_TEXCOORDS) && defined(HAS_NORMALMAPPING)
uniform sampler2D u_normalMap;
#endif

#if defined(HAS_TEXCOORDS) && defined(HAS_METALLICMAPPING)
uniform sampler2D u_metallicMap;
#endif

#if defined(HAS_TEXCOORDS) && defined(HAS_ROUGHNESSMAPPING)
uniform sampler2D u_roughnessMap;
#endif

#if defined(HAS_NORMALS) && defined(HAS_LIGHTING)
uniform int u_lightIndices[MAX_LIGHTS_PER_NODE];
#endif

#ifdef HAS_NORMALS
in vec3 v_normal;
#endif

#ifdef HAS_TEXCOORDS
in vec2 v_texCoord;
#endif

#if defined(HAS_NORMALS) && defined(HAS_TANGENTS)
in vec3 v_tangent;
in vec3 v_binormal;
#endif

#ifdef HAS_COLORS
in vec3 v_color;
#endif

#if defined(HAS_NORMALS) && defined(HAS_LIGHTING)
in vec3 v_toView;
in vec3 v_toLight[MAX_LIGHTS_PER_NODE];
in vec4 v_posLightSpace[MAX_LIGHTS_PER_NODE];
#endif

out vec4 fragColor;

void main(void)
{
    vec3 color = u_color.rgb;

#ifdef HAS_COLORS
    color *= v_color;
#endif

#if defined(HAS_TEXCOORDS) && defined(HAS_BASECOLORMAPPING)
    color *= toLinearRGB(texture(u_baseColorMap, v_texCoord).rgb);
#endif

#if defined(HAS_NORMALS) && defined(HAS_LIGHTING)
    vec3 V = normalize(v_toView);
    PbrData pbr = PbrData(color, u_metallicRoughness.x, u_metallicRoughness.y);
    vec3 N = normalize(v_normal);

#if defined(HAS_TANGENTS) &&defined(HAS_TEXCOORDS) && defined(HAS_NORMALMAPPING)
    vec3 tangent = normalize(v_tangent);
    vec3 binormal = normalize(v_binormal);
    N = normalize(mat3(tangent, binormal, N) * (texture(u_normalMap, v_texCoord).xyz * 2.0 - vec3(1.0)));
#endif    

#if defined(HAS_TEXCOORDS) && defined(HAS_METALLICMAPPING)
    pbr.metallic *= texture(u_metallicMap, v_texCoord).r;
#endif

#if defined(HAS_TEXCOORDS) && defined(HAS_ROUGHNESSMAPPING)
    pbr.roughness *= texture(u_roughnessMap, v_texCoord).r;
#endif

    const float dielectricSpecular = 0.04;
    vec3 F0 = mix(vec3(dielectricSpecular), pbr.baseColor.rgb, pbr.metallic);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS_PER_NODE; i++)
    {
        uint lightIdx = uint(u_lightIndices[i]);
        vec3 L = normalize(v_toLight[i]);

        float shadow = lightShadow(lightIdx, v_posLightSpace[i].xyz / v_posLightSpace[i].w);
        Lo += calcPbrLighting(pbr, F0, LIGHT_COLOR(u_lights[lightIdx]), N, L, V, lightAttenuation(lightIdx, v_toLight[i])) * shadow;
    }

    color = vec3(0.0);
    color += Lo;
    color += calcIblLighting(pbr, F0, N, V);
#endif

    float opacity = u_color.a;
#ifdef HAS_OPACITYMAPPING
    opacity *= texture(u_opacityMap, v_texCoord).r;
#endif

    fragColor = vec4(color, opacity);
}
