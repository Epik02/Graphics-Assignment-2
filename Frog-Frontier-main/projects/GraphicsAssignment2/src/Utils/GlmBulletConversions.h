#pragma once
#include <GLM/glm.hpp>
#include <btBulletCollisionCommon.h>

// Helper functions for fast conversions between bullet and GLM types

inline glm::vec3& ToGlm(btVector3& value) { return *reinterpret_cast<glm::vec3*>(&value); }
inline const glm::vec3& ToGlm(const btVector3& value) { return *reinterpret_cast<const glm::vec3*>(&value); }

inline btVector3& ToBt(glm::vec3& value) { return *reinterpret_cast<btVector3*>(&value); }
inline const btVector3& ToBt(const glm::vec3& value) { return *reinterpret_cast<const btVector3*>(&value); }

inline glm::quat& ToGlm(btQuaternion& value) { return *reinterpret_cast<glm::quat*>(&value); }
inline const glm::quat& ToGlm(const btQuaternion& value) { return *reinterpret_cast<const glm::quat*>(&value); }

inline btQuaternion& ToBt(glm::quat& value) { return *reinterpret_cast<btQuaternion*>(&value); }
inline const btQuaternion& ToBt(const glm::quat& value) { return *reinterpret_cast<const btQuaternion*>(&value); }
