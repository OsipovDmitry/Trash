#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_texCoord;
layout (location = 2) in vec3 a_tangent;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;
uniform vec4 u_light;

out vec3 v_tangent;
out vec3 v_binormal;
out vec3 v_normal;
out vec2 v_texCoord;
out vec3 v_toLight;

void main(void)
{
    vec4 pos = u_modelMatrix * vec4(a_position, 1.0);
    gl_Position = u_projMatrix * u_viewMatrix * pos;
    
    v_tangent = u_normalMatrix * a_tangent;
    v_normal = u_normalMatrix * a_normal;
    v_binormal = cross(v_normal, v_tangent);

    v_texCoord = a_texCoord.xy;

    v_toLight = u_light.xyz - pos.xyz;
}
