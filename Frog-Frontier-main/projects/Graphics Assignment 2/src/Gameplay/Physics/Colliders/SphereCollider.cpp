#include "SphereCollider.h"

#include "Utils/ImGuiHelper.h"

namespace Gameplay::Physics {
	SphereCollider::Sptr SphereCollider::Create(float radius /*= 1.0f*/) {
		return std::shared_ptr<SphereCollider>(new SphereCollider(radius));
	}

	SphereCollider::SphereCollider(float radius) :
		ICollider(ColliderType::Sphere),
		_radius(radius)
	{ }

	SphereCollider::~SphereCollider()= default;

	btCollisionShape* SphereCollider::CreateShape() const {
		return new btSphereShape(_radius);
	}

	SphereCollider* SphereCollider::SetRadius(float value) {
		_radius = value;
		_isDirty = true;
		return this;
	}

	float SphereCollider::GetRadius() const {
		return _radius;
	}

	void SphereCollider::FromJson(const nlohmann::json& data) {
		_radius = data["radius"];
	}

	void SphereCollider::ToJson(nlohmann::json& blob) const {
		blob["radius"] = _radius;
	}

	void SphereCollider::DrawImGui() {
		_isDirty |= LABEL_LEFT(ImGui::DragFloat, "Radius", &_radius, 0.01f);
	}
}