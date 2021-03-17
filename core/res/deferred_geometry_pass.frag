#include<gammacorrection.glsl>

uniform vec4 u_color;
uniform vec2 u_metallicRoughness;

#if defined(HAS_TEXCOORDS) && defined(HAS_BASECOLORMAPPING)
uniform sampler2D u_baseColorMap;
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

in vec3 v_normal;

#ifdef HAS_TEXCOORDS
in vec2 v_texCoord;
#endif

#if defined(HAS_TANGENTS)
in vec3 v_tangent;
in vec3 v_binormal;
#endif

#ifdef HAS_COLORS
in vec3 v_color;
#endif

out vec4 fragColor1;
out vec4 fragColor2;

void main(void)
{
    vec3 baseColor = u_color.rgb;
#ifdef HAS_COLORS
    baseColor *= v_color;
#endif
#if defined(HAS_TEXCOORDS) && defined(HAS_BASECOLORMAPPING)
    baseColor *= toLinearRGB(texture(u_baseColorMap, v_texCoord).rgb);
#endif

    float intensity = max(max(baseColor.r, baseColor.g), max(baseColor.b, 1.0));
    baseColor /= intensity;
    intensity = clamp((intensity - 1.0) / 1023.0, 0.0, 1.0);

    float metallic = u_metallicRoughness.x;
#if defined(HAS_TEXCOORDS) && defined(HAS_METALLICMAPPING)
    metallic *= texture(u_metallicMap, v_texCoord).r;
#endif

    float roughness = u_metallicRoughness.y;
#if defined(HAS_TEXCOORDS) && defined(HAS_ROUGHNESSMAPPING)
    roughness *= texture(u_roughnessMap, v_texCoord).r;
#endif

    vec3 N = normalize(v_normal);
#if defined(HAS_TEXCOORDS) && defined(HAS_NORMALMAPPING)
    vec3 tangent = normalize(v_tangent);
    vec3 binormal = normalize(v_binormal);
    N = normalize(mat3(tangent, binormal, N) * (texture(u_normalMap, v_texCoord).xyz * 2.0 - vec3(1.0)));
#endif

    fragColor1 = vec4(baseColor, roughness);
    fragColor2 = vec4(N.xy * 0.5 + vec2(0.5), intensity, metallic);
}
