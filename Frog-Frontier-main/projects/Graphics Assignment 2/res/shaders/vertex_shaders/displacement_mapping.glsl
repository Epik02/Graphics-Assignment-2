#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

// For more detailed explanations, see
// https://learnopengl.com/Advanced-Lighting/Normal-Mapping

uniform sampler2D s_Heightmap;
uniform sampler2D s_NormalMap;
uniform float u_Scale;

void main() {
    
    // Read our displacement value from the texture and apply the scale
    float displacement = textureLod(s_Heightmap, inUV, 0).r * u_Scale;
    // We'll use our surface normal for the dispalcement. We could use a normal map,
    // but this should give us OK results. Note that our displacement will be in
    // object space
    vec3 displacedPos = inPosition + (inNormal * displacement);

    // Transform to world position
	gl_Position = u_ModelViewProjection * vec4(displacedPos, 1.0);

	// Pass vertex pos in world space to frag shader
	outViewPos = (u_ModelView * vec4(displacedPos, 1.0)).xyz;

    // We use a TBN matrix for tangent space normal mapping
    vec3 T = normalize((u_View * vec4(mat3(u_NormalMatrix) * inTangent, 0)).xyz);
    vec3 B = normalize((u_View * vec4(mat3(u_NormalMatrix) * inBiTangent, 0)).xyz);
    vec3 N = normalize((u_View * vec4(mat3(u_NormalMatrix) * inNormal, 0)).xyz);
    mat3 TBN = mat3(T, B, N);

    // We can pass the TBN matrix to the fragment shader to save computation
    outTBN = TBN;

    // Read our tangent from the map, and convert from the [0,1] range to [-1,1] range
    vec3 normal = inNormal;
    
    // Here we apply the TBN matrix to transform the normal from tangent space to world space
    normal = normalize(TBN * normal);

	// Normals
	outNormal = normal;

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

}