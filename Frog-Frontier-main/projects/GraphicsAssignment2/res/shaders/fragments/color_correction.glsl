// We need the flags from the frame uniforms
#include "frame_uniforms.glsl"

// Our color correction 3d texture
uniform layout (binding=14) sampler3D s_ColorCorrection;

// Function for applying color correction
vec3 ColorCorrect(vec3 inputColor) {
    // If our color correction flag is set, we perform the color lookup
    if (IsFlagSet(FLAG_ENABLE_COLOR_CORRECTION)) {
        return texture(s_ColorCorrection, inputColor).rgb;
    }
    // Otherwise just return the input
    else {
        return inputColor;
    }
}

