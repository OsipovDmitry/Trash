#if !defined(RADIUS) || !defined(NUM_SAMPLES)
  error!!!
#endif

uniform uvec2 u_viewportSize;

uniform sampler2D u_gBufferMap0;
uniform sampler2D u_gBufferMap2;

uniform mat4 u_projMatrixInverse;
uniform mat4 u_projMatrix;

layout (std140) uniform u_ssaoSamplesBuffer
{
    vec4 u_samples[64];
};

out vec4 fragColor;

void main(void)
{
    vec2 normalizedTexCoord = gl_FragCoord.xy / vec2(u_viewportSize - 1u);

    float depth = texelFetch(u_gBufferMap0, ivec2(normalizedTexCoord * (textureSize(u_gBufferMap0, 0) - 1)), 0).r;
    vec4 normalXYIntensityMetallic = texelFetch(u_gBufferMap2, ivec2(normalizedTexCoord * (textureSize(u_gBufferMap2, 0) - 1)), 0);

    vec2 Nxy = 2.0 * normalXYIntensityMetallic.rg - vec2(1.0);
    float Nz = sqrt(1.0 - min(Nxy.x*Nxy.x + Nxy.y*Nxy.y, 1.0));
    vec3 N = vec3(Nxy, Nz);

    int x = int(gl_FragCoord.x) % 8;
    int y = int(gl_FragCoord.y) % 8;
    float noise = u_samples[8*y + x].w;
    vec3 noiseVec = vec3(cos(noise), sin(noise), 0.0);

    vec3 tangent = normalize(noiseVec - N * dot(noiseVec, N));
    vec3 binormal = cross(N, tangent);
    mat3 TBN = mat3(tangent, binormal, N);

    vec4 pixelPos = u_projMatrixInverse * vec4(2.0 * vec3(normalizedTexCoord, depth) - vec3(1.0), 1.0);
    pixelPos /= pixelPos.w;

    float occlusion = 0.0;
    for(int i = 0; i < NUM_SAMPLES; ++i)
    {
        vec3 samplePos = pixelPos.xyz + TBN * u_samples[i].xyz * RADIUS;
        vec4 offset = u_projMatrix * vec4(samplePos, 1.0);
        offset /= offset.w;
        offset.xyz = offset.xyz * 0.5 + vec3(0.5);

        float sampleDepth = texelFetch(u_gBufferMap0, ivec2(offset.xy * (textureSize(u_gBufferMap0, 0) - 1)), 0).r;
        vec4 offsetView = u_projMatrixInverse * vec4(vec3(offset.xy, sampleDepth) * 2 - vec3(1.0), 1.0);
        offsetView /= offsetView.w;

        float rangeCheck = step(abs(offsetView.z - samplePos.z), RADIUS);
        occlusion += step(sampleDepth, offset.z) * rangeCheck;
    }

    fragColor = vec4(occlusion / float(NUM_SAMPLES));
}
