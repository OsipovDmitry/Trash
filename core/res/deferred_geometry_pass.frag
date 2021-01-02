#ifdef HAS_LIGHTING
#include<pbr.glsl>
#endif

#ifdef HAS_NORMALMAPPING
uniform sampler2D u_normalMap;
#endif

uniform vec4 u_color;

#ifdef HAS_LIGHTING
in vec3 v_normal;
#ifdef HAS_NORMALMAPPING
in vec3 v_tangent;
in vec3 v_binormal;
#endif
#endif

in vec2 v_texCoord;

out vec4 fragColor1;
out vec4 fragColor2;

void main(void)
{
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec3 N = vec3(0.0, 0.0, 0.0);
    float metallic = 0.0;
    float roughness = 0.6;

#ifdef HAS_LIGHTING
    N = normalize(v_normal);

#ifdef HAS_NORMALMAPPING
    vec3 tangent = normalize(v_tangent);
    vec3 binormal = normalize(v_binormal);
    N = normalize(mat3(tangent, binormal, N) * (texture(u_normalMap, v_texCoord).xyz * 2.0 - vec3(1.0)));
#endif

    PbrData pbr = getPbrData(v_texCoord);
    color = pbr.baseColor * u_color.rgb;
    metallic = pbr.metallic;
    roughness = pbr.roughness;

#else
    color = u_color.rgb;
#endif

    fragColor1 = vec4(color, roughness);
    fragColor2 = vec4(N * 0.5 + vec3(0.5), metallic);
}
