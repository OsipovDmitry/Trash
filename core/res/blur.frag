#if !defined(RADIUS)
  error!!!
#endif

uniform uvec2 u_viewportSize;

uniform vec2 u_blurOffset;
uniform int u_blurLevel;

uniform sampler2D u_blurSourceMap;

layout (std140) uniform u_blurKernelBuffer
{
    vec4 u_kernel[16];
};

out vec4 fragColor;

void main(void)
{
    vec4 color = u_kernel[0][0] * texelFetch(u_blurSourceMap, ivec2(gl_FragCoord.xy / vec2(u_viewportSize - 1u) * (textureSize(u_blurSourceMap, u_blurLevel) - 1)), u_blurLevel);
    for (int i = 1; i < RADIUS; i++)
    {
        color += u_kernel[i/4][i%4] *
                (texelFetch(u_blurSourceMap, ivec2((gl_FragCoord.xy + float(i)*u_blurOffset) / vec2(u_viewportSize - 1u) * (textureSize(u_blurSourceMap, u_blurLevel) - 1)), u_blurLevel) +
                 texelFetch(u_blurSourceMap, ivec2((gl_FragCoord.xy - float(i)*u_blurOffset) / vec2(u_viewportSize - 1u) * (textureSize(u_blurSourceMap, u_blurLevel) - 1)), u_blurLevel));
    }

    fragColor = color;
}
