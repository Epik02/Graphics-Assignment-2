#pragma once
#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// Represents a simple box collider that has extends around the origin point
	/// </summary>
	class BoxCollider final : public ICollider {
	public:
		typedef std::shared_ptr<BoxCollider> Sptr;

		/// <summary>
		/// Create a new box collider with the given extents
		/// </summary>
		/// <param name="extents">The size of the box collider, will be centered around shape's origin</param>
		static BoxCollider::Sptr Create(const glm::vec3& extents = glm::vec3(1.0f));
		virtual ~BoxCollider();

		/// <summary>
		/// Gets the extents of this box collider
		/// </summary>
		const glm::vec3& GetExtents() const;
		/// <summary>
		/// Updates the extents of this collider and marks it
		/// as dirty
		/// </summary>
		/// <param name="value">The new extents for the collider</param>
		void SetExtents(const glm::vec3& value);

		// Inherited from ICollider
		virtual void DrawImGui() override;
		virtual void ToJson(nlohmann::json& blob) const override;
		virtual void FromJson(const nlohmann::json& data) override;

	protected:
		BoxCollider(const glm::vec3& extents);
		glm::vec3 _extents;

		virtual btCollisionShape* CreateShape() const override;
	};
}