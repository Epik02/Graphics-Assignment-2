#version 440

#include "../fragments/fs_common_inputs.glsl"

layout(location = 4) in mat3 inTBN;

// We output a single color to the color buffer
layout(location = 0) out vec4 frag_color;

////////////////////////////////////////////////////////////////
/////////////// Frame Level Uniforms ///////////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/frame_uniforms.glsl"

////////////////////////////////////////////////////////////////
/////////////// Instance Level Uniforms ////////////////////////
////////////////////////////////////////////////////////////////

// Represents a collection of attributes that would define a material
// For instance, you can think of this like material settings in 
// Unity
struct Material {
	sampler2D Diffuse;
	sampler2D Emissive;
	sampler2D NormalMap;
	float     Shininess;
};
// Create a uniform for the material
uniform Material u_Material;

uniform sampler2D s_NormalMap;

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
    
    // Read our tangent from the map, and convert from the [0,1] range to [-1,1] range
    vec3 normal = texture(s_NormalMap, inUV).rgb;
    normal = normal * 2.0 - 1.0;
    
    // Here we apply the TBN matrix to transform the normal from tangent space to world space
    normal = normalize(inTBN * normal);

	// Will accumulate the contributions of all lights on this fragment
	// This is defined in the fragment file "multiple_point_lights.glsl"
	vec3 lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	// combine for the final result
	vec3 result = lightAccumulation  * inColor * textureColor.rgb;

	frag_color = vec4(ColorCorrect(result), textureColor.a);
}