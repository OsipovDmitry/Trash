#if !defined(HAS_POSITIONS) || !defined(MAX_LIGHTS_PER_NODE)
  error!!!
#endif

layout (location = 0) in vec3 a_position;

#ifdef HAS_NORMALS
layout (location = 1) in vec3 a_normal;
uniform mat3 u_normalMatrix;
#endif

#ifdef HAS_TEXCOORDS
layout (location = 2) in vec2 a_texCoord;
#endif

#ifdef HAS_BONES
layout (location = 3) in vec4 a_boneIDs;
layout (location = 4) in vec4 a_boneWeights;
layout (std140) uniform u_bonesBuffer { mat3x4 u_bones[128]; };
#endif

#if defined(HAS_NORMALS) && defined(HAS_TANGENTS)
layout (location = 5) in vec3 a_tangent;
#endif

#ifdef HAS_COLORS
layout (location = 6) in vec3 a_color;
#endif

uniform mat4 u_viewProjMatrix;
uniform mat4 u_modelMatrix;

#if defined(HAS_NORMALS) && defined(HAS_LIGHTING)
#include<lights.glsl>
uniform int u_lightIndices[MAX_LIGHTS_PER_NODE];
uniform vec3 u_viewPosition;
#endif

#ifdef HAS_NORMALS
out vec3 v_normal;
#endif

#ifdef HAS_TEXCOORDS
out vec2 v_texCoord;
#endif

#if defined(HAS_NORMALS) && defined(HAS_TANGENTS)
out vec3 v_tangent;
out vec3 v_binormal;
#endif

#ifdef HAS_COLORS
out vec3 v_color;
#endif

#if defined(HAS_NORMALS) && defined(HAS_LIGHTING)
out vec3 v_toView;
out vec3 v_toLight[MAX_LIGHTS_PER_NODE];
out vec4 v_posLightSpace[MAX_LIGHTS_PER_NODE];
#endif

void main(void)
{
    vec4 pos = vec4(a_position, 1.0);

#ifdef HAS_BONES
    mat3x4 bonesMatrix =
            u_bones[int(a_boneIDs[0])] * a_boneWeights[0] +
            u_bones[int(a_boneIDs[1])] * a_boneWeights[1] +
            u_bones[int(a_boneIDs[2])] * a_boneWeights[2] +
            u_bones[int(a_boneIDs[3])] * a_boneWeights[3];

    pos = vec4(pos * bonesMatrix, 1.0);
#endif

    pos = u_modelMatrix * pos;
    gl_Position = u_viewProjMatrix * pos;

#ifdef HAS_NORMALS
    v_normal = normalize(a_normal);
#ifdef HAS_BONES
    v_normal = normalize(vec4(v_normal, 0.0) * bonesMatrix);
#endif
    v_normal = u_normalMatrix * v_normal;
#endif

#ifdef HAS_TEXCOORDS
    v_texCoord = a_texCoord;
#endif

#if defined(HAS_NORMALS) && defined(HAS_TANGENTS)
    v_tangent = normalize(a_tangent);
#ifdef HAS_BONES
    v_tangent = normalize(vec4(v_tangent, 0.0) * bonesMatrix);
#endif
    v_tangent = normalize(u_normalMatrix * v_tangent);
    v_binormal = normalize(cross(v_normal, v_tangent));
#endif

#ifdef HAS_COLORS
    v_color = a_color;
#endif

#if defined(HAS_NORMALS) && defined(HAS_LIGHTING)
    v_toView = u_viewPosition - pos.xyz;
    for (int i = 0; i < MAX_LIGHTS_PER_NODE; i++)
    {
        v_toLight[i] = (u_lightIndices[i] >= 0) ? toLightVector(uint(u_lightIndices[i]), pos.xyz) : vec3(0.0, 0.0, 0.0);
        v_posLightSpace[i] = (u_lightIndices[i] >= 0) ? LIGHT_MATRIX(u_lights[u_lightIndices[i]]) * pos : vec4(0.0, 0.0, 0.0, 1.0);
    }
#endif



}
