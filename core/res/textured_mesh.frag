#version 330 core

//--------LIGHTS------------

#define Light mat4

#define LIGHT_POSITION(light) (light[0].xyz)
#define LIGHT_DIRECTION(light) (light[1].xyz)
#define LIGHT_COLOR(light) (light[2].xyz)
#define LIGHT_ATTENUATION(light) (light[3].xyz)
#define LIGHT_SPOT_COS_INNER(light) (light[0].w)
#define LIGHT_SPOT_COS_OUTER(light) (light[1].w)
#define LIGHT_TYPE(light) (int(light[2][3] + 0.5))

#define LIGHT_TYPE_NONE (0)
#define LIGHT_TYPE_POINT (1)
#define LIGHT_TYPE_DIRECTION (2)
#define LIGHT_TYPE_SPOT (3)
#define LIGHT_TYPE_COUNT (4)

const float s_isDirectionLightType[LIGHT_TYPE_COUNT] = float[LIGHT_TYPE_COUNT](0.0, 0.0, 1.0, 0.0);
const float s_distanceAttenuationLightType[LIGHT_TYPE_COUNT] = float[LIGHT_TYPE_COUNT](0.0, 1.0, 0.0, 1.0);
const float s_spotAttenuationLightType[LIGHT_TYPE_COUNT] = float[LIGHT_TYPE_COUNT](0.0, 0.0, 0.0, 1.0);

vec3 toLightVector(in Light light, vec3 v) {
    return mix(LIGHT_POSITION(light) - v, -LIGHT_DIRECTION(light), s_isDirectionLightType[LIGHT_TYPE(light)]);
}

//---------------------------

uniform sampler2D u_diffuseMap;
uniform sampler2D u_normalMap;

in vec3 v_tangent;
in vec3 v_binormal;
in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_toLight;

out vec4 fragColor;

void main(void)
{
    vec3 tangent = normalize(v_tangent);
    vec3 binormal = normalize(v_binormal);
    vec3 normal = normalize(v_normal);
    vec3 toLight = normalize(v_toLight);

    vec3 fragNormal = mat3(tangent, binormal, normal) * vec3(texture(u_normalMap, v_texCoord).xyz * 2.0 - vec3(1.0, 1.0, 1.0));

    vec3 diffuseColor = texture(u_diffuseMap, v_texCoord).xyz;
    float diffuse = max(0.0, dot(toLight, fragNormal));

    fragColor = vec4(diffuseColor * diffuse, 1.0);
}
