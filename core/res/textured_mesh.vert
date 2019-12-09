#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_texCoord;
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;

uniform mat4 u_projMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

layout (std140) uniform u_bonesBuffer
{
    mat3x4 u_bones[100];
};

out vec3 v_normal;
out vec2 v_texCoord;

void main(void)
{
    mat3x4 bonesMatrix =
            u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
            u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
            u_bones[int(a_boneIDs[2])] * a_boneWeights[2] +
            u_bones[int(a_boneIDs[3])] * a_boneWeights[3];

    vec3 pos = vec4(a_position, 1.0) * bonesMatrix;
    v_normal = u_normalMatrix * (vec4(a_normal, 0.0) * bonesMatrix);

    gl_Position = u_projMatrix * u_viewMatrix * u_modelMatrix * vec4(pos, 1.0);
    
    v_texCoord = a_texCoord.xy;
}
