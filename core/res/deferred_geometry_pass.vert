layout (location = 0) in vec3 a_position;
layout (location = 2) in vec3 a_texCoord;

#ifdef HAS_LIGHTING
layout (location = 1) in vec3 a_normal;
#ifdef HAS_NORMALMAPPING
layout (location = 5) in vec3 a_tangent;
#endif
#endif

#ifdef HAS_BONES
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;
#endif

uniform mat4 u_modelViewProjMatrix;

#ifdef HAS_LIGHTING
uniform mat3 u_normalMatrix;
#endif

#ifdef HAS_BONES
layout (std140) uniform u_bonesBuffer
{
    mat3x4 u_bones[128];
};
#endif

#ifdef HAS_LIGHTING
out vec3 v_normal;
#ifdef HAS_NORMALMAPPING
out vec3 v_tangent;
out vec3 v_binormal;
#endif
#endif

out vec2 v_texCoord;

void main(void)
{
    vec4 pos = vec4(a_position, 1.0);

    mat3x4 bonesMatrix;

#ifdef HAS_BONES
    bonesMatrix =
            u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
            u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
            u_bones[int(a_boneIDs[2])] * a_boneWeights[2] +
            u_bones[int(a_boneIDs[3])] * a_boneWeights[3];

    pos = vec4(pos * bonesMatrix, 1.0);
#endif

    gl_Position = u_modelViewProjMatrix * pos;

#ifdef HAS_LIGHTING
    v_normal = normalize(a_normal);
#ifdef HAS_BONES
    v_normal = normalize(vec4(v_normal, 0.0) * bonesMatrix);
#endif
    v_normal = u_normalMatrix * v_normal;

#ifdef HAS_NORMALMAPPING
    v_tangent = normalize(a_tangent);
#ifdef HAS_BONES
    v_tangent = normalize(vec4(v_tangent, 0.0) * bonesMatrix);
#endif
    v_tangent = normalize(u_normalMatrix * v_tangent);
    v_binormal = normalize(cross(v_normal, v_tangent));
#endif
#endif
    
    v_texCoord = a_texCoord.xy;
}
