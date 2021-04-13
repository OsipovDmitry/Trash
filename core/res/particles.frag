in vec4 v_color;
in vec2 v_offset;

out vec4 fragColor;

void main(void)
{
    float attenuation = 1.0 - min(1.0, length(v_offset));

    vec4 color = v_color;
    color.a *= attenuation;

    fragColor = color;
}
