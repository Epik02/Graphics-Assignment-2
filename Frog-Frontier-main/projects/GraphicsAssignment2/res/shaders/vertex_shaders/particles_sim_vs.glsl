#version 440

layout (location = 0) in uint  inType;
layout (location = 1) in vec3  inPosition;
layout (location = 2) in vec3  inVelocity;
layout (location = 3) in vec4  inColor;
layout (location = 4) in float inLifetime;
layout (location = 5) in vec4  inMetadata;

layout (location = 0) out uint  outType; 
layout (location = 1) out vec3  outPosition;
layout (location = 2) out vec3  outVelocity;
layout (location = 3) out vec4  outColor;
layout (location = 4) out float outLifetime;
layout (location = 5) out vec4  outMetadata;

// Simple passthrough to the geometry shader
void main() {
    outType     = inType;
    outPosition = inPosition;
    outVelocity = inVelocity;
    outColor    = inColor;
    outLifetime = inLifetime;
    outMetadata = inMetadata;
}

