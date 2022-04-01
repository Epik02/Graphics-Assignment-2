#pragma once
#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// A collider for cylinder along the Z axis
	/// </summary>
	class CylinderCollider final : public ICollider {
	public:
		typedef std::shared_ptr<CylinderCollider> Sptr;
		static CylinderCollider::Sptr Create(const glm::vec3& halfExtents = glm::vec3(0.5f, 0.5f, 1.0f));
		virtual ~CylinderCollider();

		CylinderCollider* SetHalfExtents(const glm::vec3& value);
		const glm::vec3& GetHalfExtents() const;


		// Inherited from ICollider
		virtual void DrawImGui() override;
		virtual void ToJson(nlohmann::json& blob) const override;
		virtual void FromJson(const nlohmann::json& data) override;

	protected:
		virtual btCollisionShape* CreateShape() const override;

	private:
		glm::vec3 _extents;

		CylinderCollider(const glm::vec3& extents);
	};
}