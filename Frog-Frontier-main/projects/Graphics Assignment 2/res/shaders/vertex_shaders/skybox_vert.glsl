#version 440

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outNormal;

uniform mat4 u_ClippedView;
uniform mat3 u_EnvironmentRotation;

void main() {
    vec4 pos = u_ClippedView * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;

    // Normals
    outNormal = normalize(u_EnvironmentRotation * inPosition);
}