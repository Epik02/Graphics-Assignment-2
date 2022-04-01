#include "Utils/GlmDefines.h"

glm::mat4 MAT4_IDENTITY = glm::mat4(1.0f);
glm::mat3 MAT3_IDENTITY = glm::mat3(1.0f);
glm::vec4 UNIT_X = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 UNIT_Y = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
glm::vec4 UNIT_Z = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
glm::vec4 UNIT_W = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

glm::vec3 UNIT_X_3 = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 UNIT_Y_3 = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 UNIT_Z_3 = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 UNIT_W_3 = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec4 ZERO = glm::vec4(0.0f);
glm::vec3 ZERO_3 = glm::vec3(0.0f);
glm::vec2 ZERO_2 = glm::vec2(0.0f);
glm::vec4 ONE = glm::vec4(1.0f);
glm::vec3 ONE_3 = glm::vec3(1.0f);
glm::vec2 ONE_2 = glm::vec2(1.0f);
glm::vec3 DEGREE_MAX = glm::vec3(360.0f);
glm::vec2 DEGREE_MAX_2 = glm::vec2(360.0f);

void NormalizeScaleRef(glm::mat4& transform) {
	// We can normalize the 3x3 part for the matrix to remove the scaling from the transform!
	for (int ix = 0; ix < 3; ix++) {
		float w = transform[ix][3];
		glm::vec3 n = glm::normalize((glm::vec3)transform[ix]);
		transform[ix] = glm::vec4(n, w);
	}
}

glm::mat4 NormalizeScale(const glm::mat4& transform) {
	glm::mat4 result = transform;
	NormalizeScaleRef(result);
	return result;
}
