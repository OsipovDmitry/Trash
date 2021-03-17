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
#define LIGHT_IS_SHADOW_ENABLED(light) (abs(light.params[3][3]) > 0.5)
#define LIGHT_IS_SHADOW_OUTSIDE(light) (light.params[3][3] > 0.0)
#define LIGHT_MATRIX(light) (light.matrix)

#define LIGHT_TYPE_POINT (0)
#define LIGHT_TYPE_DIRECTION (1)
#define LIGHT_TYPE_SPOT (2)

#define MAX_LIGHTS 8

const int pcfRadius = 2;

vec3 toLightVector(in uint lightIdx, vec3 v)
{
    LightStruct light = u_lights[lightIdx];

    int type = LIGHT_TYPE(light);
    if ((type == LIGHT_TYPE_POINT) || (type == LIGHT_TYPE_SPOT))
        return LIGHT_POSITION(light) - v;
    else if (type == LIGHT_TYPE_DIRECTION)
        return -LIGHT_DIRECTION(light);
    else
        return vec3(0.0, 0.0, 0.0);
}

float lightAttenuation(in uint lightIdx, vec3 toLight)
{
    LightStruct light = u_lights[lightIdx];

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

float lightShadow(in uint lightIdx, in vec3 posInLightSpace)
{
    LightStruct light = u_lights[lightIdx];

    float shadow = 0.0;

    if (LIGHT_IS_SHADOW_ENABLED(light) == false)
    {
        shadow = 1.0;
    }
    else
    {
        vec2 shadowMapSize = textureSize(u_shadowMaps, 0).xy;

        for (int x = -pcfRadius; x <= pcfRadius; ++x)
            for (int y = -pcfRadius; y <= pcfRadius; ++y)
            {
                if (any(lessThan(posInLightSpace, vec3(0.0))) || any(greaterThan(posInLightSpace, vec3(1.0))))
                    shadow += LIGHT_IS_SHADOW_OUTSIDE(light) ? 0.0 : 1.0;
                else
                    shadow += texture(u_shadowMaps, vec4(posInLightSpace.xy + vec2(x,y) / shadowMapSize, float(lightIdx), posInLightSpace.z));
            }
        shadow = shadow / ((2*pcfRadius+1) * (2*pcfRadius+1));
    }

    return shadow;
}
