const float gamma = 2.2;

vec3 toLinearRGB(in vec3 color) {
    return pow(color, vec3(gamma));
}

vec3 toSRGB(in vec3 color) {
    return pow(color, vec3(1.0/gamma));
}

vec4 toLinearRGB(in vec4 color) {
    return vec4(pow(color.rgb, vec3(gamma)), color.a);
}

vec4 toSRGB(in vec4 color) {
    return vec4(pow(color.rgb, vec3(1.0/gamma)), color.a);
}
