#include "CapsuleCollider.h"

#include "Utils/ImGuiHelper.h"

namespace Gameplay::Physics {
	CapsuleCollider::Sptr CapsuleCollider::Create(float radius /*= 0.5f*/, float height /*= 1.0f*/) {
		return std::shared_ptr<CapsuleCollider>(new CapsuleCollider(radius, height));
	}

	CapsuleCollider::CapsuleCollider(float radius, float height) :
		ICollider(ColliderType::Capsule),
		_radius(radius),
		_height(height)
	{ }

	CapsuleCollider::~CapsuleCollider() = default;

	void CapsuleCollider::DrawImGui() {
		_isDirty |= LABEL_LEFT(ImGui::DragFloat, "Radius", &_radius, 0.1f, 0.01f);
		_isDirty |= LABEL_LEFT(ImGui::DragFloat, "Height", &_radius, 0.1f, 0.01f);
	}

	void CapsuleCollider::ToJson(nlohmann::json& blob) const {
		blob["radius"] = _radius;
		blob["height"] = _height;
	}

	void CapsuleCollider::FromJson(const nlohmann::json& data)
	{
		_radius = data["radius"];
		_height = data["height"];
	}

	btCollisionShape* CapsuleCollider::CreateShape() const {
		return new btCapsuleShapeZ(_radius, _height);
	}


	CapsuleCollider* CapsuleCollider::SetRadius(float value) {
		_radius = value;
		_isDirty = true;
		return this;
	}

	float CapsuleCollider::GetRadius() const {
		return _radius;
	}

	CapsuleCollider* CapsuleCollider::SetHeight(float value) {
		_height = value;
		_isDirty = true;
		return this;
	}

	float CapsuleCollider::GetHeight() const {
		return _height;
	}
}