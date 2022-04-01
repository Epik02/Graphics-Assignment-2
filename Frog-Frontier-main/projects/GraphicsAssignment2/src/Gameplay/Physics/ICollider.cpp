#include "Gameplay/Physics/ICollider.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

// Utils
#include "Utils/GlmDefines.h"

// Collider Types
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/CapsuleCollider.h"
#include "Gameplay/Physics/Colliders/ConeCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"

namespace Gameplay::Physics {
	const char* ColliderTypeComboNames = "Plane\0Box\0Sphere\0Capsule\0Cone\0Cylinder\0Convex Mesh\0Concave Mesh\0Terrain\0";

	ICollider::ICollider(ColliderType type) :
		_type(type),
		_shape(nullptr),
		_position(glm::vec3(0.0f)),
		_rotation(glm::vec3(0.0f)),
		_scale(glm::vec3(1.0f)),
		_guid(Guid::New())
	{ }

	ICollider::~ICollider() {
		if (_shape != nullptr) {
			delete _shape;
			_shape = nullptr;
		}
	}

	ColliderType ICollider::GetType() const {
		return _type;
	}

	btCollisionShape* ICollider::GetShape() const {
		if (_shape == nullptr) {
			_shape = CreateShape();
		}
		return _shape;
	}

	ICollider* ICollider::SetPosition(const glm::vec3& value) {
		_position = value;
		_isDirty  = true;
		return this;
	}

	const glm::vec3& ICollider::GetPosition() const {
		return _position;
	}

	ICollider* ICollider::SetRotation(const glm::vec3& value) {
		_rotation = glm::fmod(value, DEGREE_MAX);
		_isDirty = true;
		return this;
	}

	const glm::vec3& ICollider::GetRotation() const {
		return _rotation;
	}

	ICollider* ICollider::SetScale(const glm::vec3& value) {
		_scale = value;
		_isDirty = true;
		return this;
	}

	const glm::vec3& ICollider::GetScale() const {
		return _scale;
	}

	Guid ICollider::GetGUID() const {
		return _guid;
	}

	ICollider::Sptr ICollider::Create(ColliderType type) {
		switch (type)
		{
			case ColliderType::Plane:       return PlaneCollider::Create();
			case ColliderType::Box:         return BoxCollider::Create();
			case ColliderType::Sphere:      return SphereCollider::Create();
			case ColliderType::Capsule:     return CapsuleCollider::Create();
			case ColliderType::Cone:        return ConeCollider::Create();
			case ColliderType::Cylinder:    return CylinderCollider::Create();
			case ColliderType::ConvexMesh:  return ConvexMeshCollider::Create();
			case ColliderType::ConcaveMesh: throw std::runtime_error("Collider type not supported!"); return nullptr;
			case ColliderType::Terrain:     throw std::runtime_error("Collider type not supported!"); return nullptr;
			case ColliderType::Unknown:
			default:
				return nullptr;
		}

	}
}