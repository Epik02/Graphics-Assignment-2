#version 440

layout(location = 0) in vec3 inNormal;

uniform layout (binding=15) samplerCube s_Environment;

// We output a single color to the color buffer
layout(location = 0) out vec4 albedo_specPower;
layout(location = 1) out vec4 normal_metallic;
layout(location = 2) out vec4 emissive;
layout(location = 3) out vec3 view_pos;

void main() {
    vec3 norm = normalize(inNormal);

    albedo_specPower = vec4(texture(s_Environment, norm).rgb, 0.0);
    normal_metallic = vec4(0.5, 0.5, 1, 0);
    emissive = vec4(0);
    view_pos = vec3(0);
}