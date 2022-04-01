
// Vertex inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBiTangent;

// Standard vertex shader outputs
layout(location = 0) out vec3 outViewPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;
layout(location = 4) out mat3 outTBN;

// Include the matrices and frame level parameters
#include "frame_uniforms.glsl"
