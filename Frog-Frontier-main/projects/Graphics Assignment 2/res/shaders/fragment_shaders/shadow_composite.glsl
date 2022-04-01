#version 450

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;

// Note the use of sampler2DShadow here! This lets us perform
// linear sampling on a depth buffer (more or less)
layout (binding = 5) uniform sampler2DShadow s_ShadowDepth;

// Image to project
layout (binding = 6) uniform sampler2D s_ProjectionMask;

// Matrix to go from view space to shadow clip space
uniform mat4  u_ViewToShadow;
// Light's direction in view space
uniform vec3  u_LightDirViewspace;
// Light's position in view space
uniform vec3  u_LightPosViewspace;

// Shadow settings
uniform float u_ShadowBias;
uniform float u_NormalBias;
uniform uint  u_ShadowFlags;

// Light settings
uniform float u_Attenuation;
uniform float u_Intensity;
uniform vec3  u_LightColor;

// Flags
#define FLAG_PROJECTION_ENABLED (1 << 0)
#define FLAG_ENABLE_PCF (1 << 1)
#define FLAG_ENABLE_ATTENUATION (1 << 2)
#define FLAG_ENABLE_WIDE_PCF (1 << 3)

/*
 * Determines if one of the shadow option flags is set,
 * if multiple flags are provided, checks all of them
 */
bool ShadowFlagSet(uint flag) {
    return (u_ShadowFlags & flag) == flag;
}

// Represents a single light source
struct Light {
	vec4  PositionIntensity;
	// Stores color in RBG and attenuation in w
	vec4  ColorAttenuation;
};

#include "../fragments/deferred_post_common.glsl"
#include "../fragments/frame_uniforms.glsl"

// Showing off another way to extract view pos from depth
vec4 GetViewPos(vec2 uv) {
	// Get the depth buffer value at this pixel, map from [0,1] to [-1,1]    
	float zOverW = texture(s_Depth, uv).r * 2 - 1;
	// We convert the range [0,1] to [-1,1], create a point to inverse project    
	vec4 currentPos = vec4(uv.xy * 2 - 1, zOverW, 1);
	// Transform by the view-projection inverse    
	vec4 D = inverse(u_Projection) * currentPos;
	// Divide by w for perspective divide    
	vec4 viewPos = D / D.w;
	return viewPos;
}

// Calculates the contribution the given point light has 
// for the current fragment
// @param viewPos   The fragment's position in view space
// @param normal    The fragment's normal (normalized)
// @param Light     The light to caluclate the contribution for
// @param shininess The specular power for the fragment, between 0 and 1
void CalcDirectionalLightContribution(vec3 viewPos, vec3 normal, Light light, float shininess, inout vec3 diffuse, inout vec3 specular) {

        vec3 lightViewPos = light.PositionIntensity.xyz;
        vec3 lightVec = lightViewPos - viewPos;
        float dist = length(lightVec);
        vec3 lightDir = -u_LightDirViewspace;

        float attenuation = 1.0;
        // We'll use a modified distance squared attenuation factor to keep it simple
        // We add the one to prevent divide by zero errors
        if (ShadowFlagSet(FLAG_ENABLE_ATTENUATION)) {
            attenuation = clamp(1.0 / (1.0 + light.ColorAttenuation.w * pow(dist, 2)), 0, 256);
        }

        // Dot product between normal and light
        float NdotL = max(dot(normal, lightDir), 0.0);
        diffuse += NdotL * attenuation * light.PositionIntensity.w * light.ColorAttenuation.rgb;
        
        vec3 reflectDir = reflect(lightDir, normal);
        float VdotR = pow(max(dot(normalize(-viewPos), reflectDir), 0.0), pow(2, shininess * 8));
        
        specular += VdotR * light.ColorAttenuation.rgb * shininess * attenuation * light.PositionIntensity.w;
}

// This function will sample multiple points around our sample, and average the results
// This gives a slight blur to the edges of the shadows, and helps to soften them up
// @param fragPos The position in the shadow's normalized clip space to sample
// @param bias The shadow bias factor to use
float PCF(vec3 fragPos, float bias) {

    // If we're doing PCF, we want to take multiple samples
    if (ShadowFlagSet(FLAG_ENABLE_PCF)) {
        float result = 0.0; // accumulator
        vec2 texelSize = 1.0 / textureSize(s_ShadowDepth, 0); // Determine the texel size of the shadow sampler
        
        // 5x5 kernel
        if (ShadowFlagSet(FLAG_ENABLE_WIDE_PCF)) {
            // Normalized 5x5 gaussian kernel
            const float kernel[5][5] = {
                { 1.0/273,  4.0/273,  7.0/273,  4.0/273, 1.0/273 },
                { 4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273 },
                { 7.0/273, 26.0/273, 41.0/273, 26.0/273, 7.0/273 },
                { 4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273 },
                { 1.0/273,  4.0/273,  7.0/273,  4.0/273, 1.0/273 },
            };

            // Iterate over a 5x5 area of texels around our sample location
            for(int x = -2; x <= 2; ++x) { 
                for(int y = -2; y <= 2; ++y) {
                    // Note the use of a vec3 for sample pos! The z is the depth to compare,
                    // OpenGL will take care of the rest and return a value between 0 and 1
                    // as long as the texture is a sampler2DShadow. This is also where bias is
                    // applied.
                    float contrib =
                        texture(
                            s_ShadowDepth, 
                            vec3(fragPos.xy + vec2(x,y) * texelSize, fragPos.z - bias)
                        );
                    // Apply kernel weights to the result
                    result += contrib * kernel[x+2][y+2];
                }    
            }
        }
        // 3x3 kernel
        else {
            // Normalized 3x3 gaussian kernel
            const float kernel[3][3] = {
                { 1.0/16, 2.0/16, 1.0/16 },
                { 2.0/16, 4.0/16, 2.0/16 },
                { 1.0/16, 2.0/16, 1.0/16 }
            };

            // Iterate over a 3x3 area of texels around our sample location
            for(int x = -1; x <= 1; ++x) { 
                for(int y = -1; y <= 1; ++y) {
                    // See above notes about texture
                    float contrib = texture(s_ShadowDepth, vec3(fragPos.xy + vec2(x,y) * texelSize, fragPos.z - bias));
                    result += contrib * kernel[x+1][y+1];
                }    
            }
        }

        return result;
    }
    // PCF is not enabled, take 1 sample
    else {
        // See above notes about texture
        float contrib = texture(s_ShadowDepth, vec3(fragPos.xy, fragPos.z - bias));
        return contrib; // Perform the depth test, and return the result
    }
}

void main() {
    // Normal of sample in view space
    vec3 normal = GetNormal(inUV);
    
    // Ignore things we can't calculate light for
    if (length(normal) < 0.1) {
        discard;
    }

    // Make sure the normal is in fact, a normal
    normal = normalize(normal);

    // Get values from the g-buffer
    vec3 albedo = GetAlbedo(inUV);

    // Get viewspace from depth re-construction method (just to show how it works!)
    vec3 viewPos = GetViewPos(inUV).xyz;

    // Determine the position in light clip space
	vec4 shadowPos = u_ViewToShadow * vec4(viewPos, 1.0);  
	shadowPos /= shadowPos.w;                // Perspective divide
	shadowPos = shadowPos * 0.5 + 0.5;       // Normalize from clip space to [0,1]
    
    // If pixel on screen is outside the bounds of the light, skip it
    if (shadowPos.x < 0 || shadowPos.x > 1 || 
        shadowPos.y < 0 || shadowPos.y > 1 || 
        shadowPos.z < 0 || shadowPos.z > 1) {
        //outDiffuse  = vec4(1, 0, 0, 1);
        //outSpecular = vec4(1, 0, 0, 1);
        //return;
        discard;
    }

    // Calculate a bias based on the dot product between surface normal and light direction
    float bias = max(u_NormalBias * (1.0 - dot(normal, u_LightDirViewspace)), u_ShadowBias);

    // Determine how much of the pixel on the screen is in shadow
    float lightContrib = PCF(shadowPos.xyz, bias);

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    // We can skip lighting calculation if the pixel is fully in shadow!
    if (lightContrib > 0) {

        // Create a light structure we can pass to the CalcDirectionalLightContribution function
        Light l;
        l.PositionIntensity = vec4(u_LightPosViewspace, u_Intensity);

        // If we want to use the projection mask, we sample it and multiply by light color
        if (ShadowFlagSet(FLAG_PROJECTION_ENABLED)) {
            vec3 color = texture(s_ProjectionMask, shadowPos.xy).rgb * u_LightColor;
            l.ColorAttenuation = vec4(color, u_Attenuation);
        }
        // We do not want to use the projection mask, just use the light color
        else {
            l.ColorAttenuation = vec4(u_LightColor, u_Attenuation);
        }

        // We'll also grab specular power from the G-Buffer
        float specularPow = texture(s_AlbedoSpec, inUV).a;

        // Use the structure to calculate a directional light's contribution
        CalcDirectionalLightContribution(viewPos, normal, l, specularPow, diffuse, specular);

        // We multiply the final light contribution by the inverse of the shadow
        diffuse  *= lightContrib;
        specular *= lightContrib;
    }

    // Return our results
    outDiffuse = vec4(diffuse, 1);
    outSpecular = vec4(specular, 1);
}