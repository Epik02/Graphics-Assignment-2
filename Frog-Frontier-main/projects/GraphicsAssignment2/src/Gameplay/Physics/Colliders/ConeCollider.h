#pragma once
#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// Represents a cone collider collider with a radius and a height. The
	/// cone will point towards +Z
	/// </summary>
	class ConeCollider final : public ICollider {
	public:
		typedef std::shared_ptr<ConeCollider> Sptr;
		static ConeCollider::Sptr Create(float radius = 0.5f, float height = 1.0f);
		virtual ~ConeCollider();

		/// <summary>
		/// Sets the radius of the bottom of the cone, in meters
		/// </summary>
		/// <param name="value">The new radius for the cone</param>
		/// <returns>A pointer to the collider, for operator chaining only</returns>
		ConeCollider* SetRadius(float value);
		/// <summary>
		/// Gets the radius of the bottom of the cone in meters
		/// </summary>
		float GetRadius() const;

		/// <summary>
		/// Sets the height of the cone, in meters
		/// </summary>
		/// <param name="value">The new height for the cone</param>
		/// <returns>A pointer to the collider, for operator chaining only</returns>
		ConeCollider* SetHeight(float value);
		/// <summary>
		/// Gets the height of the cone in meters
		/// </summary>
		float GetHeight() const;

		// Inherited from ICollider

		virtual void DrawImGui() override;
		virtual void ToJson(nlohmann::json& blob) const override;
		virtual void FromJson(const nlohmann::json& data) override;

	protected:
		virtual btCollisionShape* CreateShape() const override;

	private:
		float _radius;
		float _height;

		ConeCollider(float radius, float height);
	};
}