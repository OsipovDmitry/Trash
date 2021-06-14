
#ifdef PARTICLE_DISTANCE_ATTENUATION
uniform mat4 u_projMatrixInverse;
uniform uvec2 u_viewportSize;
uniform float u_particleDistanceAttenuation;
uniform sampler2D u_gBufferMap0;
#endif

in vec4 v_color;
in vec2 v_offset;
flat in int v_isAlive;

#ifdef PARTICLE_DISTANCE_ATTENUATION
in float v_viewDepth;
#endif

#ifdef HAS_OPACITYMAPPING
uniform sampler2DArray u_opacityMap;
in vec2 v_texCoord;
flat in vec2 v_frameNumber;
#endif

out vec4 fragColor;

void main(void)
{
    if (v_isAlive == 0)
        discard;

    float attenuation = 1.0;
#if defined(PARTICLE_TYPE_QUAD)
#elif defined(PARTICLE_TYPE_CIRCLE)
    attenuation *= step(length(v_offset), 1.0);
#elif defined(PARTICLE_TYPE_SOFT_CIRCLE)
    attenuation *= 1.0 - min(1.0, length(v_offset));
#endif

#ifdef PARTICLE_DISTANCE_ATTENUATION
    vec2 normalizedTexCoord = gl_FragCoord.xy / vec2(u_viewportSize - 1u);
    float depth = texelFetch(u_gBufferMap0, ivec2(normalizedTexCoord * (textureSize(u_gBufferMap0, 0) - 1)), 0).r;

    vec4 pixelViewPos = u_projMatrixInverse * vec4(2.0 * vec3(normalizedTexCoord, depth) - vec3(1.0), 1.0);
    pixelViewPos /= pixelViewPos.w;

    float contr = min(u_particleDistanceAttenuation, abs(pixelViewPos.z - v_viewDepth)) / u_particleDistanceAttenuation;
    attenuation *= contr * contr;
#endif

#ifdef HAS_OPACITYMAPPING
    int prevFrameNumber = int(v_frameNumber.x + 0.5);
    int nextFrameNumber = (prevFrameNumber + 1) % textureSize(u_opacityMap, 0)[2];

    attenuation *= mix(texture(u_opacityMap, vec3(v_texCoord, float(prevFrameNumber))).r,
                       texture(u_opacityMap, vec3(v_texCoord, float(nextFrameNumber))).r,
                       v_frameNumber.y);
#endif

    vec4 color = v_color;
    color.a *= attenuation;

    fragColor = color;
}
