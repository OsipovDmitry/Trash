#version 330 core

#include<lights.glsl>

#define MAX_LIGHTS 8

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_texCoord;
layout (location = 2) in vec3 a_tangent;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_viewPosition;
uniform int u_lightIndices[MAX_LIGHTS];

layout (std140) uniform u_lightsBuffer
{
    LightStruct u_lights[128];
};

out vec3 v_tangent;
out vec3 v_binormal;
out vec3 v_normal;
out vec2 v_texCoord;
out vec3 v_toView;
out vec3 v_toLight[MAX_LIGHTS];
out vec4 v_posLightSpace[MAX_LIGHTS];

void main(void)
{
    vec4 pos = u_modelMatrix * vec4(a_position, 1.0);
    gl_Position = u_projMatrix * u_viewMatrix * pos;
    
    v_tangent = u_normalMatrix * a_tangent;
    v_normal = u_normalMatrix * a_normal;
    v_binormal = cross(v_normal, v_tangent);

    v_texCoord = a_texCoord.xy;
    v_toView = u_viewPosition - pos.xyz;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        v_toLight[i] = (u_lightIndices[i] >= 0) ? toLightVector(u_lights[u_lightIndices[i]], pos.xyz) : vec3(0.0, 0.0, 0.0);
        v_posLightSpace[i] = (u_lightIndices[i] >= 0) ? LIGHT_MATRIX(u_lights[u_lightIndices[i]]) * pos : vec4(0.0, 0.0, 0.0, 1.0);
    }
}
