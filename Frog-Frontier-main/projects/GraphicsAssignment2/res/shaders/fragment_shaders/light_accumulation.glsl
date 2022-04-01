#version 440

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;

// The maximum number of lights the shader supports, increasing this will lower performance!
#define MAX_LIGHTS 8

// Represents a single light source
struct Light {
	vec4  PositionIntensity;
	// Stores color in RBG and attenuation in w
	vec4  ColorAttenuation;
};

// Our uniform buffer that will store all our lighting data
// so that it can be shared between shaders
layout (std140, binding = 2) uniform b_LightBlock {
    // Stores ambient light color in rgb, and number
	// of lights in w, allowing for easier struct packing
	// on the C++ side
    vec4  AmbientColAndNumLights;

    // Our array of all lights
    Light Lights[MAX_LIGHTS];

    // The rotation of the skybox/environment map
	mat3  EnvironmentRotation;
};

#include "../fragments/deferred_post_common.glsl"

#include "../fragments/frame_uniforms.glsl"

// Calculates the contribution the given point light has 
// for the current fragment
// @param viewPos   The fragment's position in view space
// @param normal    The fragment's normal (normalized)
// @param Light     The light to caluclate the contribution for
// @param shininess The specular power for the fragment, between 0 and 1
void CalcPointLightContribution(vec3 viewPos, vec3 normal, Light light, float shininess, inout vec3 diffuse, inout vec3 specular) {

        vec3 lightViewPos = light.PositionIntensity.xyz;
        vec3 lightVec = lightViewPos - viewPos;
        float dist = length(lightVec);
        vec3 lightDir = lightVec / dist;

        // We'll use a modified distance squared attenuation factor to keep it simple
        // We add the one to prevent divide by zero errors
        float attenuation = clamp(1.0 / (1.0 + light.ColorAttenuation.w * pow(dist, 2)), 0, 256);

        // Dot product between normal and light
        float NdotL = max(dot(normal, lightDir), 0.0);
        diffuse += NdotL * attenuation * light.PositionIntensity.w * light.ColorAttenuation.rgb;
        
        vec3 reflectDir = reflect(lightDir, normal);
        float VdotR = pow(max(dot(normalize(-viewPos), reflectDir), 0.0), pow(2, shininess * 8));
        
        specular += VdotR * light.ColorAttenuation.rgb * shininess * attenuation * light.PositionIntensity.w;
}

void main() {
    vec3 normal = GetNormal(inUV);
    
    if (length(normal) < 0.1) {
        discard;
    }

    normal = normalize(normal);

    vec3 albedo = GetAlbedo(inUV);
    vec3 viewPos = GetViewPosition(inUV);
    
    float specularPow = texture(s_AlbedoSpec, inUV).a;

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
    for (int ix = 0; ix < AmbientColAndNumLights.w && ix < MAX_LIGHTS; ix++) {
        CalcPointLightContribution(viewPos, normal, Lights[ix], specularPow, diffuse, specular);
    }

    outDiffuse = vec4(diffuse, 1);
    outSpecular = vec4(specular, 1);
}