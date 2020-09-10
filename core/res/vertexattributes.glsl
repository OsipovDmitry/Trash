const float gamma = 2.2;

vec3 toLinearRGB(in vec3 color) {
    return pow(color, vec3(gamma));
}

vec3 toSRGB(in vec3 color) {
    return pow(color, vec3(1.0/gamma));
}
