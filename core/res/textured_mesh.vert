#version 330 core

#include<:/res/lights.glsl>

#define MAX_LIGHTS 8

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_texCoord;
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;
layout (location = 5) in vec3 a_tangent;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_viewPosition;
uniform int u_lightIndices[MAX_LIGHTS];

layout (std140) uniform u_bonesBuffer
{
    mat3x4 u_bones[128];
};

layout (std140) uniform u_lightsBuffer
{
    mat4x4 u_lights[128];
};

out vec3 v_tangent;
out vec3 v_binormal;
out vec3 v_normal;
out vec2 v_texCoord;
out vec3 v_toView;
out vec3 v_toLight[MAX_LIGHTS];

void main(void)
{
    mat3x4 bonesMatrix =
            u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
            u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
            u_bones[int(a_boneIDs[2])] * a_boneWeights[2] +
            u_bones[int(a_boneIDs[3])] * a_boneWeights[3];

    v_normal = u_normalMatrix * (vec4(a_normal, 0.0) * bonesMatrix);
    v_tangent = u_normalMatrix * (vec4(a_tangent, 0.0) * bonesMatrix);
    v_binormal = cross(v_normal, v_tangent);

    vec4 pos = u_modelMatrix * vec4((vec4(a_position, 1.0) * bonesMatrix), 1.0);
    gl_Position = u_projMatrix * u_viewMatrix * pos;
    
    v_texCoord = a_texCoord.xy;
    v_toView = u_viewPosition - pos.xyz;

    for (int i = 0; i < MAX_LIGHTS; i++)
        v_toLight[i] = (u_lightIndices[i] >= 0) ? toLightVector(u_lights[u_lightIndices[i]], pos.xyz) : vec3(0.0, 0.0, 0.0);
}
