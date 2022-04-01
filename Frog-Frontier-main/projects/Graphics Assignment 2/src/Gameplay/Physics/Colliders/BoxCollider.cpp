#include "Gameplay/Physics/Colliders/BoxCollider.h"

// Utils
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/GlmDefines.h"

namespace Gameplay::Physics {
	BoxCollider::Sptr BoxCollider::Create(const glm::vec3& extents /*= glm::vec3(1.0f)*/) {
		return std::shared_ptr<BoxCollider>(new BoxCollider(extents));
	}

	BoxCollider::BoxCollider(const glm::vec3& extents) :
		ICollider(ColliderType::Box),
		_extents(extents)
	{ }

	BoxCollider::~BoxCollider() = default;

	btCollisionShape* BoxCollider::CreateShape() const {
		return new btBoxShape(btVector3(_extents.x, _extents.y, _extents.z));
	}

	void BoxCollider::FromJson(const nlohmann::json& data) {
		_extents = data["extents"];
	}

	void BoxCollider::ToJson(nlohmann::json& blob) const {
		blob["extents"] = _extents;
	}

	void BoxCollider::DrawImGui() {
		_isDirty |= LABEL_LEFT(ImGui::DragFloat3, "Extents  ", &_extents.x, 0.01f, 0.01f);
	}

	const glm::vec3& BoxCollider::GetExtents() const {
		return _extents;
	}

	void BoxCollider::SetExtents(const glm::vec3& value) {
		_extents = value;
		_isDirty = true;
	}
}