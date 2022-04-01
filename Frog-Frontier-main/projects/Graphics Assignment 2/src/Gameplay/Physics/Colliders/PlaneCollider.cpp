#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/GlmDefines.h"

namespace Gameplay::Physics {
	PlaneCollider::Sptr PlaneCollider::Create(const glm::vec3& normal /*= glm::vec3(0.0f, 0.0f, 1.0f)*/) {
		return std::shared_ptr<PlaneCollider>(new PlaneCollider(normal));
	}

	PlaneCollider::PlaneCollider(const glm::vec3& normal) :
		ICollider(ColliderType::Plane),
		_normal(normal)
	{ }

	PlaneCollider::~PlaneCollider() = default;

	void PlaneCollider::DrawImGui() {
		_isDirty |= LABEL_LEFT(ImGui::DragFloat3, "Normal", &_normal.x, 0.01f, -1.0f, 1.0f);
	}

	void PlaneCollider::ToJson(nlohmann::json& blob) const {
		blob["normal"] = (_normal);
	}

	void PlaneCollider::FromJson(const nlohmann::json& data) {
		_normal = (data["normal"]);
	}

	btCollisionShape* PlaneCollider::CreateShape() const {
		return new btStaticPlaneShape(btVector3(_normal.x, _normal.y, _normal.z), 0.0f);
	}

	const glm::vec3& PlaneCollider::GetNormal() const {
		return _normal;
	}

	void PlaneCollider::SetNormal(const glm::vec3& value) {
		_normal = value;
		_isDirty = true;
	}
}