layout (location = 0) in vec4 a_position;
layout (location = 2) in vec2 a_texCoord;
layout (location = 6) in vec4 a_color;

uniform mat4 u_modelViewProjMatrix;
uniform vec3 u_viewXDirection;
uniform vec3 u_viewYDirection;

out vec4 v_color;
out vec2 v_offset;

void main(void)
{
    vec3 pos = a_position.xyz;
    float scale = a_position.w;

    v_color = a_color;
    v_offset = a_texCoord * 2.0 - 1.0;

    pos += scale * (v_offset.x * u_viewXDirection + v_offset.y * u_viewYDirection);
    gl_Position = u_modelViewProjMatrix * vec4(pos, 1.0);
}
