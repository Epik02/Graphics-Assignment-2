#include "ConeCollider.h"

#include "Utils/ImGuiHelper.h"

namespace Gameplay::Physics {
	ConeCollider::Sptr ConeCollider::Create(float radius /*= 0.5f*/, float height /*= 1.0f*/) {
		return std::shared_ptr<ConeCollider>(new ConeCollider(radius, height));
	}

	ConeCollider::ConeCollider(float radius, float height) :
		ICollider(ColliderType::Cone),
		_radius(radius),
		_height(height)
	{ }

	ConeCollider::~ConeCollider() = default;

	void ConeCollider::DrawImGui() {
		_isDirty |= LABEL_LEFT(ImGui::DragFloat, "Radius", &_radius, 0.1f, 0.01f);
		_isDirty |= LABEL_LEFT(ImGui::DragFloat, "Height", &_radius, 0.1f, 0.01f);
	}

	void ConeCollider::ToJson(nlohmann::json & blob) const {
		blob["radius"] = _radius;
		blob["height"] = _height;
	}

	void ConeCollider::FromJson(const nlohmann::json & data)
	{
		_radius = data["radius"];
		_height = data["height"];
	}

	btCollisionShape* ConeCollider::CreateShape() const {
		return new btConeShapeZ(_radius, _height);
	}


	ConeCollider* ConeCollider::SetRadius(float value) {
		_radius = value;
		_isDirty = true;
		return this;
	}

	float ConeCollider::GetRadius() const {
		return _radius;
	}

	ConeCollider* ConeCollider::SetHeight(float value) {
		_height = value;
		_isDirty = true;
		return this;
	}

	float ConeCollider::GetHeight() const {
		return _height;
	}
}
