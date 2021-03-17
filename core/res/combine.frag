uniform uvec2 u_viewportSize;

uniform int u_combineLevel0;
uniform int u_combineLevel1;

uniform sampler2D u_combineSourceMap0;
uniform sampler2D u_combineSourceMap1;

out vec4 fragColor;

void main(void)
{
    vec2 normalizedTexCoord = gl_FragCoord.xy / vec2(u_viewportSize - 1u);

    vec4 color;

#if defined(COMBINE_ADD)
    color = texelFetch(u_combineSourceMap0, ivec2(normalizedTexCoord * (textureSize(u_combineSourceMap0, u_combineLevel0) - 1)), u_combineLevel0) +
            texelFetch(u_combineSourceMap1, ivec2(normalizedTexCoord * (textureSize(u_combineSourceMap1, u_combineLevel1) - 1)), u_combineLevel1);
#endif

    fragColor = color;
}
