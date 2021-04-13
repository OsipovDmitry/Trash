#include<gammacorrection.glsl>

uniform uvec2 u_viewportSize;

uniform sampler2D u_hdrMap;
uniform sampler2D u_bloomMap;

out vec4 fragColor;

void main(void)
{
    vec2 normalizedTexCoord = gl_FragCoord.xy / vec2(u_viewportSize - 1u);

    vec3 color = vec3(0.0);
    color += texelFetch(u_hdrMap, ivec2(normalizedTexCoord * (textureSize(u_hdrMap, 0) - 1)), 0).rgb;
    color += texelFetch(u_bloomMap, ivec2(normalizedTexCoord * (textureSize(u_bloomMap, 0) - 1)), 0).rgb;

    color = color / (color + vec3(1.0));
    color = toSRGB(color);

    fragColor = vec4(color, 1.0);
}
