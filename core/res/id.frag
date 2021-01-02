uniform uint u_id;

out uvec4 fragColor;

void main(void)
{
    fragColor = uvec4(u_id, 0, 0, 0);
}
