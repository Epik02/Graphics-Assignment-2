#pragma once
#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// Simple sphere collider with a radius
	/// </summary>
	class SphereCollider final : public ICollider {
	public:
		typedef std::shared_ptr<SphereCollider> Sptr;
		static SphereCollider::Sptr Create(float radius = 1.0f);
		virtual ~SphereCollider();

		SphereCollider* SetRadius(float value);
		float GetRadius() const;

		// Inherited from ICollider
		virtual void DrawImGui() override;
		virtual void ToJson(nlohmann::json& blob) const override;
		virtual void FromJson(const nlohmann::json& data) override;

	protected:
		virtual btCollisionShape* CreateShape() const override;

	private:
		float _radius;

		SphereCollider(float radius);
	};
}