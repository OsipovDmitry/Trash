layout (location = 0) in vec3 a_position;

#ifdef HAS_BONES
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;
#endif

uniform mat4 u_modelViewProjMatrix;

#ifdef HAS_BONES
layout (std140) uniform u_bonesBuffer
{
    mat3x4 u_bones[128];
};
#endif

void main(void)
{
    vec4 pos = vec4(a_position, 1.0);

#ifdef HAS_BONES
        mat3x4 bonesMatrix;
        bonesMatrix =
                u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
                u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
                u_bones[int(a_boneIDs[2])] * a_boneWeights[2] +
                u_bones[int(a_boneIDs[3])] * a_boneWeights[3];

        pos = vec4(pos * bonesMatrix, 1.0);
#endif

    gl_Position = u_modelViewProjMatrix * pos;
}
