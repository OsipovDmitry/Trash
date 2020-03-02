struct LightStruct
{
    mat4x4 params;
    mat4x4 matrix;
};

#define LIGHT_POSITION(light) (light.params[0].xyz)
#define LIGHT_DIRECTION(light) (light.params[1].xyz)
#define LIGHT_COLOR(light) (light.params[2].xyz)
#define LIGHT_ATTENUATION(light) (light.params[3].xyz)
#define LIGHT_SPOT_COS_INNER(light) (light.params[0].w)
#define LIGHT_SPOT_COS_OUTER(light) (light.params[1].w)
#define LIGHT_TYPE(light) (int(light.params[2][3] + 0.5))
#define LIGHT_IS_SHADOW_ENABLED(light) (light.params[3][3] > 0.0)
#define LIGHT_MATRIX(light) (light.matrix)


#define LIGHT_TYPE_NONE (0)
#define LIGHT_TYPE_POINT (1)
#define LIGHT_TYPE_DIRECTION (2)
#define LIGHT_TYPE_SPOT (3)
#define LIGHT_TYPE_COUNT (4)

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
        float dist = length(toLight);
        vec3 l = normalize(toLight);
        vec3 attCoef = LIGHT_ATTENUATION(light);

        att *= 1.0 / (attCoef.x * dist * dist + attCoef.y * dist + attCoef.z);

        if (type == LIGHT_TYPE_SPOT)
        {
            float cosAngle = dot(-l, LIGHT_DIRECTION(light));
            float spotAtt = (cosAngle - LIGHT_SPOT_COS_OUTER(light)) / (LIGHT_SPOT_COS_INNER(light) - LIGHT_SPOT_COS_OUTER(light));
            spotAtt = clamp(spotAtt, 0.0, 1.0);
            att *= spotAtt;
        }
    }

    return att;
}
