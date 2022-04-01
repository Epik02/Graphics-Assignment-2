#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

// Attributes 0-5 are used by our common inputs, so let's skip to 8 to leave some space
// This will consume 4 slots, since it's essentially 4 vec4s in memory
layout(location = 8) in mat4 inModelTransform;
// This will consume 3 slots in memory
layout(location = 12) in mat3 inNormalMatrix;

void main() {
	// We take the hit of doing a matrix multiplication instead of using more bandwidth to send all the matrices
	gl_Position = (u_ViewProjection * inModelTransform) * vec4(inPosition, 1.0); 

	// Lecture 5
	// Pass vertex pos in world space to frag shader
	outWorldPos = (inModelTransform * vec4(inPosition, 1.0)).xyz;

	// Normals
	outNormal = mat3(inNormalMatrix) * inNormal;

    // We use a TBN matrix for tangent space normal mapping
    vec3 T = normalize(vec3(mat3(inNormalMatrix) * inTangent));
    vec3 B = normalize(vec3(mat3(inNormalMatrix) * inBiTangent));
    vec3 N = normalize(vec3(mat3(inNormalMatrix) * inNormal));
    mat3 TBN = mat3(T, B, N);

    // We can pass the TBN matrix to the fragment shader to save computation
    outTBN = TBN;

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

}

