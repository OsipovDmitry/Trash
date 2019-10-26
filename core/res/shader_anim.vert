#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

layout (std140) uniform u_ModelData
{
    mat3x4 u_bones[100];
};

out vec3 v_normal;
out vec2 v_texCoord;

void main(void)
{
    vec3 pos = vec4(a_position, 1.0) * (u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
               u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
               u_bones[int(a_boneIDs[2])] * a_boneWeights[2] + 
               u_bones[int(a_boneIDs[3])] * a_boneWeights[3]);

    gl_Position = u_projMatrix * u_viewMatrix * u_modelMatrix * vec4(pos, 1.0);
    v_normal = a_normal;
    v_texCoord = a_texCoord;
}