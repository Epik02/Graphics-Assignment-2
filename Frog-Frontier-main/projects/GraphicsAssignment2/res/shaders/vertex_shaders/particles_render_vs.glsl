#version 450

layout (location = 0) in uint  inType;
layout (location = 1) in vec3  inPosition;
layout (location = 3) in vec4  inColor;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out flat uint outType;
layout (location = 2) out vec3 viewPos;

#include "../fragments/frame_uniforms.glsl"

void main() {
    viewPos = (u_View * vec4(inPosition, 1)).xyz;
    gl_Position = u_Projection * vec4(viewPos, 1);
    fragColor = inColor;
    outType = inType;
    gl_PointSize = 10.0; 
}