#version 430

#include "../fragments/fs_common_inputs.glsl"
#include "../fragments/frame_uniforms.glsl"

// We output a single color to the color buffer
layout(location = 0) out vec4 albedo_specPower;
layout(location = 1) out vec4 normal_metallic;
layout(location = 2) out vec4 emissive;
layout(location = 3) out vec3 view_pos;

// Represents a collection of attributes that would define a material
// For instance, you can think of this like material settings in 
// Unity
struct Material {
	sampler2D AlbedoMap;
	sampler2D EmissiveMap;
	sampler2D NormalMap;
	sampler2D MetallicShininessMap;
	float     DiscardThreshold;
};
// Create a uniform for the material
uniform Material u_Material;

uniform sampler1D s_ToonTerm;

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {	
	// Get albedo from the material
	vec4 albedoColor = texture(u_Material.AlbedoMap, inUV);

    // Using a LUT to allow artists to tweak toon shading settings
    albedoColor.r = texture(s_ToonTerm, albedoColor.r).r;
    albedoColor.g = texture(s_ToonTerm, albedoColor.g).g;
    albedoColor.b = texture(s_ToonTerm, albedoColor.b).b;

	// We can use another texture to store things like our lighting settings
	vec4 lightingParams = texture(u_Material.MetallicShininessMap, inUV);

	// Discarding fragments who's alpha is below the material's threshold
	if (albedoColor.a < u_Material.DiscardThreshold) {
		discard;
	}

	// Extract albedo from material, and store shininess
	albedo_specPower = vec4(albedoColor.rgb, lightingParams.x);
	
	// Normalize our input normal
    // Read our tangent from the map, and convert from the [0,1] range to [-1,1] range
    vec3 normal = texture(u_Material.NormalMap, inUV).rgb;
    normal = normal * 2.0 - 1.0;

    // Here we apply the TBN matrix to transform the normal from tangent space to view space
    normal = normalize(inTBN * normal);
	
	// Map [-1, 1] to [0, 1]
	normal = clamp((normal + 1) / 2.0, 0, 1);
	normal_metallic = vec4(normal, lightingParams.y);

	// Extract emissive from the material
	emissive = texture(u_Material.EmissiveMap, inUV);
	
	view_pos = inViewPos;
}