#pragma once
#include "GLM/glm.hpp"

extern glm::mat4 MAT4_IDENTITY;
extern glm::mat3 MAT3_IDENTITY;
extern glm::vec4 UNIT_X;
extern glm::vec4 UNIT_Y;
extern glm::vec4 UNIT_Z;
extern glm::vec4 UNIT_W;
extern glm::vec3 UNIT_X_3;
extern glm::vec3 UNIT_Y_3;
extern glm::vec3 UNIT_Z_3;
extern glm::vec3 UNIT_W_3;
extern glm::vec4 ZERO;
extern glm::vec3 ZERO_3;
extern glm::vec2 ZERO_2;
extern glm::vec4 ONE;
extern glm::vec3 ONE_3;
extern glm::vec2 ONE_2;
extern glm::vec3 DEGREE_MAX;
extern glm::vec2 DEGREE_MAX_2;

/// <summary>
/// Removes scaling from a glm matrix, modifying the value in place
/// </summary>
/// <param name="transform">The GLM transform to modify</param>
void NormalizeScaleRef(glm::mat4& transform);
/// <summary>
/// Removes scaling from a glm matrix, returning a modified copy
/// </summary>
/// <param name="transform">The GLM transform to copy</param>
/// <returns>A copy of transform with scaling normalized</returns>
glm::mat4 NormalizeScale(const glm::mat4& transform);

template <typename T, typename V>
T Wrap(const T& x, const V& min, const V& max) {
	return glm::mod((glm::mod((x - min), (max - min)) + (max - min)), (max - min)) + min;
}