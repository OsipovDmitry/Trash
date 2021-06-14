layout (location = 0) in vec4 a_position;
layout (location = 2) in vec2 a_texCoord;
layout (location = 6) in vec4 a_color;

uniform mat4 u_modelViewMatrix;
uniform mat4 u_projMatrix;
uniform vec3 u_viewXDirection;
uniform vec3 u_viewYDirection;

out vec4 v_color;
out vec2 v_offset;
flat out int v_isAlive;

#ifdef PARTICLE_DISTANCE_ATTENUATION
out float v_viewDepth;
#endif

#ifdef HAS_OPACITYMAPPING
layout (location = 3) in vec2 a_frameNumber;
out vec2 v_texCoord;
flat out vec2 v_frameNumber;
#endif

void main(void)
{
    vec4 pos = vec4(a_position.xyz, 1.0);
    float scale = a_position.w;

    v_color = a_color;
    v_offset = a_texCoord * 2.0 - 1.0;
    v_isAlive = scale > 0.0 ? 1 : 0;

    pos.xyz += scale * (v_offset.x * u_viewXDirection + v_offset.y * u_viewYDirection);

    pos = u_modelViewMatrix * pos;
    gl_Position = u_projMatrix * pos;

#ifdef PARTICLE_DISTANCE_ATTENUATION
    v_viewDepth = pos.z;
#endif

#ifdef HAS_OPACITYMAPPING
    v_texCoord = a_texCoord;
    v_frameNumber = a_frameNumber;
#endif
}
