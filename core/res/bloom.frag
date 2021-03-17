uniform uvec2 u_viewportSize;

uniform sampler2D u_hdrMap;

out vec4 fragColor;

const vec3 intensityCoefs = vec3(0.2126, 0.7152, 0.0722);

void main(void)
{
    vec3 color = texelFetch(u_hdrMap, ivec2(gl_FragCoord.xy / vec2(u_viewportSize - 1u) * (textureSize(u_hdrMap, 0) - 1)), 0).rgb;

    fragColor = vec4(color * step(1.0, dot(color, intensityCoefs)), 1.0);
}
