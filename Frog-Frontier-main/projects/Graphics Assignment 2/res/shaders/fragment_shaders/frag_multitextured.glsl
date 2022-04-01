#version 440

#include "../fragments/fs_common_inputs.glsl"

layout(location = 7) in vec4 inTextureWeights;

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
	sampler2D DiffuseA;
	sampler2D DiffuseB;
	sampler2D EmissiveA;
	sampler2D EmissiveB;
	sampler2D NormalMapA;
	sampler2D NormalMapB;
	float     Shininess;
	float     DiscardThreshold;
};
// Create a uniform for the material
uniform Material u_Material;

////////////////////////////////////////////////////////////////
///////////// Application Level Uniforms ///////////////////////
////////////////////////////////////////////////////////////////

// We output a single color to the color buffer
layout(location = 0) out vec4 albedo_specPower;
layout(location = 1) out vec4 normal_metallic;
layout(location = 2) out vec4 emissive;
layout(location = 3) out vec3 view_pos;

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Get albedo from the material
	vec4 albedoColor = 
		texture(u_Material.DiffuseA, inUV) * inTextureWeights.x +
		texture(u_Material.DiffuseB, inUV) * inTextureWeights.y;
	

	// Discarding fragments who's alpha is below the material's threshold
	if (albedoColor.a < u_Material.DiscardThreshold) {
		discard;
	}

	// Extract albedo from material, and store shininess
	albedo_specPower = vec4(albedoColor.rgb, u_Material.Shininess);
	
	// Normalize our input normal
	vec3 normal = normalize(
		texture(u_Material.NormalMapA, inUV).rgb * inTextureWeights.x +
		texture(u_Material.NormalMapA, inUV).rgb * inTextureWeights.y
	);
    normal = normal * 2.0 - 1.0;
	
    // Here we apply the TBN matrix to transform the normal from tangent space to view space
    normal = normalize(inTBN * normal);
	
	// Map [-1, 1] to [0, 1]
	normal = clamp((normal + 1) / 2.0, 0, 1);
	normal_metallic = vec4(normal, 0.0f);

	// Extract emissive from the material
	emissive = 
		texture(u_Material.EmissiveA, inUV).rgba * inTextureWeights.x +
		texture(u_Material.EmissiveB, inUV).rgba * inTextureWeights.y;
		
	view_pos = inViewPos;
}