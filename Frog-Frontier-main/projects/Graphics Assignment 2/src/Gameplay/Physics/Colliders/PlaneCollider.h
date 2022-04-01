#pragma once
#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// Represents a plane collider that has an infinately wide plane
	/// that points towards a normal at a given distance from the shape's
	/// origin
	/// </summary>
	class PlaneCollider final : public ICollider {
	public:
		typedef std::shared_ptr<PlaneCollider> Sptr;
		static PlaneCollider::Sptr Create(const glm::vec3& normal = glm::vec3(0.0f, 0.0f, 1.0f));
		virtual ~PlaneCollider();

		/// <summary>
		/// Gets the extents of this box collider
		/// </summary>
		const glm::vec3& GetNormal() const;
		/// <summary>
		/// Updates the normal of this collider and marks it
		/// as dirty
		/// </summary>
		/// <param name="value">The new normal for the collider</param>
		void SetNormal(const glm::vec3& value);

		// Inherited from ICollider
		virtual void DrawImGui() override;
		virtual void ToJson(nlohmann::json& blob) const override;
		virtual void FromJson(const nlohmann::json& data) override;

	protected:
		PlaneCollider(const glm::vec3& normal);

		glm::vec3 _normal;
		virtual btCollisionShape* CreateShape() const override;
	};
}