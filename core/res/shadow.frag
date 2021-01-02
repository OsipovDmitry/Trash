#ifdef DEBUG
out vec4 fragColor;
#endif

void main(void)
{
#ifdef DEBUG
    fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
#endif
}
