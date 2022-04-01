#include "CylinderCollider.h"

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/GlmBulletConversions.h"

namespace Gameplay::Physics {
	CylinderCollider::Sptr CylinderCollider::Create(const glm::vec3& halfextents) {
		return std::shared_ptr<CylinderCollider>(new CylinderCollider(halfextents));
	}

	CylinderCollider::CylinderCollider(const glm::vec3& halfExtents) :
		ICollider(ColliderType::Cylinder),
		_extents(halfExtents)
	{ }

	CylinderCollider::~CylinderCollider() = default;

	void CylinderCollider::DrawImGui() {
		_isDirty |= LABEL_LEFT(ImGui::DragFloat3, "Half Extents", &_extents.x, 0.1f, 0.01f);
	}

	void CylinderCollider::ToJson(nlohmann::json & blob) const {
		blob["half_extents"] = (_extents);
	}

	void CylinderCollider::FromJson(const nlohmann::json & data)
	{
		_extents = (data["half_extents"]);
	}

	btCollisionShape* CylinderCollider::CreateShape() const {
		return new btCylinderShapeZ(ToBt(_extents));
	}

	CylinderCollider* CylinderCollider::SetHalfExtents(const glm::vec3 & value) {
		_extents = value;
		_isDirty = true;
		return this;
	}

	const glm::vec3& CylinderCollider::GetHalfExtents() const {
		return _extents;
	}
}