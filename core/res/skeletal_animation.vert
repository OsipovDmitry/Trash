#version 330 core

layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;
layout (location = 6) in vec3 a_tPosition;

layout (std140) uniform u_bonesBuffer
{
    mat3x4 u_bones[100];
};

out vec3 v_position;

void main(void)
{
    v_position = vec4(a_tPosition, 1.0) * (
               u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
               u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
               u_bones[int(a_boneIDs[2])] * a_boneWeights[2] + 
               u_bones[int(a_boneIDs[3])] * a_boneWeights[3]
               );
}
