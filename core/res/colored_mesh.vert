#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

layout (std140) uniform u_bonesBuffer
{
    mat3x4 u_bones[128];
};

void main(void)
{
    mat3x4 bonesMatrix =
            u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
            u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
            u_bones[int(a_boneIDs[2])] * a_boneWeights[2] +
            u_bones[int(a_boneIDs[3])] * a_boneWeights[3];

    vec4 pos = u_modelMatrix * vec4((vec4(a_position, 1.0) * bonesMatrix), 1.0);
    gl_Position = u_projMatrix * u_viewMatrix * pos;
}
