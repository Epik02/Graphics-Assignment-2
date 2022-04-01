#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in flat uint outType;
layout (location = 2) in vec3 viewPos;

layout(location = 0) out vec4 albedo_specPower;
layout(location = 1) out vec4 normal_metallic;
layout(location = 2) out vec4 emissive;
layout(location = 3) out vec3 view_pos;

#define TYPE_EMITTER 0
#define TYPE_PARTICLE 1

void main() { 
	if (outType == TYPE_EMITTER) {
		discard;
	}

	albedo_specPower = fragColor;
	normal_metallic = vec4(0.5, 0.5, 1, 0);
	emissive = vec4(0);
	view_pos = viewPos;
}

