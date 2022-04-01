#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

uniform layout(binding = 0) sampler2D s_Sampler;

uniform bool IgnoreAlpha;

void main() {
	if (IgnoreAlpha) {
		outColor = vec4(texture(s_Sampler, inUV).rgb, 1);
	}
	else {
		outColor = texture(s_Sampler, inUV);
	}
}