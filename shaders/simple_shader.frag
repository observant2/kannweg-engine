#version 450

layout(location = 0) out vec4 outColor;

vec3 colors[3] = {
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0),
};

void main() {
    outColor = vec4(0.5, 0.5, 0.0, 1.0);
}
