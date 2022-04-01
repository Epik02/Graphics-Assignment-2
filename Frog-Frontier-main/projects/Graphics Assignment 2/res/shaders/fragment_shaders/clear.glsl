#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor[8];

uniform vec4 ClearColors[8];

void main() {
	for(int ix = 0; ix < 8; ix++) {
		outColor[ix] = ClearColors[ix];
	}
	gl_FragDepth = 1.0;
}