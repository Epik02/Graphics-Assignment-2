#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

uniform vec3  u_WindDirection;
uniform float u_WindStrength;
uniform float u_VerticalScale;
uniform float u_WindSpeed;

void main() {
    // Determine the offset based on our simple wind calcualtion
    vec3 windFactor = normalize(u_WindDirection) * sin(u_Time * u_WindSpeed) * cos(inPosition.z * u_VerticalScale) * u_WindStrength;
	// Calculate the output world position
	outViewPos = (u_ModelView * vec4(inPosition, 1.0)).xyz + windFactor;
    // Project the world position to determine the screenspace position
	gl_Position = u_Projection * vec4(outViewPos, 1);

	// Normals
	outNormal = mat3(u_NormalMatrix) * normalize(inNormal);
	
    // We use a TBN matrix for tangent space normal mapping
    vec3 T = normalize(vec3(mat3(u_NormalMatrix) * normalize(inTangent)));
    vec3 B = normalize(vec3(mat3(u_NormalMatrix) * normalize(inBiTangent)));
    vec3 N = normalize(vec3(mat3(u_NormalMatrix) * normalize(inNormal)));
    mat3 TBN = mat3(T, B, N);

	outTBN = TBN * mat3(u_View);

	// Pass our UV coords to the fragment shader
	outUV = inUV;
	outColor = inColor;
}

