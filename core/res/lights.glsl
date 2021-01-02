struct LightStruct
{
    mat4x4 params;
    mat4x4 matrix;
};

layout (std140) uniform u_lightsBuffer
{
    LightStruct u_lights[128];
};

uniform sampler2DArrayShadow u_shadowMaps;

#define LIGHT_POSITION(light) (light.params[0].xyz)
#define LIGHT_DIRECTION(light) (light.params[1].xyz)
#define LIGHT_COLOR(light) (light.params[2].xyz)
#define LIGHT_RADIUSES(light) (light.params[3].xy)
#define LIGHT_SPOT_COS_INNER(light) (light.params[0].w)
#define LIGHT_SPOT_COS_OUTER(light) (light.params[1].w)
#define LIGHT_TYPE(light) (int(light.params[2][3] + 0.5))
#define LIGHT_IS_SHADOW_ENABLED(light) (light.params[3][3] > 0.0)
#define LIGHT_MATRIX(light) (light.matrix)

#define LIGHT_TYPE_POINT (0)
#define LIGHT_TYPE_DIRECTION (1)
#define LIGHT_TYPE_SPOT (2)

#define MAX_LIGHTS 8

vec3 toLightVector(in LightStruct light, vec3 v)
{
    int type = LIGHT_TYPE(light);
    if ((type == LIGHT_TYPE_POINT) || (type == LIGHT_TYPE_SPOT))
        return LIGHT_POSITION(light) - v;
    else if (type == LIGHT_TYPE_DIRECTION)
        return -LIGHT_DIRECTION(light);
    else
        return vec3(0.0, 0.0, 0.0);
}

float lightAttenuation(in LightStruct light, vec3 toLight)
{
    float att = 1.0;
    int type = LIGHT_TYPE(light);

    if ((type == LIGHT_TYPE_POINT) || (type == LIGHT_TYPE_SPOT))
    {
        vec2 radiuses = LIGHT_RADIUSES(light);

        float dist = length(toLight);
        att *= 1.0 - smoothstep(radiuses.x, radiuses.x + radiuses.y, dist);

        if (type == LIGHT_TYPE_SPOT)
        {
            vec3 l = normalize(toLight);
            float cosAngle = dot(-l, LIGHT_DIRECTION(light));
            att *= smoothstep(LIGHT_SPOT_COS_OUTER(light), LIGHT_SPOT_COS_INNER(light), cosAngle);
        }
    }

    return att;
}

float lightShadow(in vec3 posInLightSpace, in int lightIdx)
{
    const int N = 2;

    vec2 shadowMapSize = textureSize(u_shadowMaps, 0).xy;
    float shadow = 0.0;
    int x, y;
    for (x = -N; x <= N; ++x)
        for (y = -N; y <= N; ++y)
            shadow += texture(u_shadowMaps, vec4(posInLightSpace.xy + vec2(x,y) / shadowMapSize, float(lightIdx), posInLightSpace.z));

    return shadow / ((2*N+1) * (2*N+1));
}
