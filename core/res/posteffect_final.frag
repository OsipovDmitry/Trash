#include<gammacorrection.glsl>

uniform sampler2D u_hdrMap;

uniform uvec2 u_viewportSize;

out vec4 fragColor;

void main(void)
{
    vec2 texCoord = (gl_FragCoord.xy) / vec2(u_viewportSize);

    vec3 color = texture(u_hdrMap, texCoord).rgb;

    //color = color / (color + vec3(1.0));
    color = toSRGB(color);

    fragColor = vec4(color, 1.0);
}
