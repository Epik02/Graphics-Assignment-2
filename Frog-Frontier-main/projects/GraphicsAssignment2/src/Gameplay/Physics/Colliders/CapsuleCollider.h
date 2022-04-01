#pragma once
#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// Represents a capsule collider with a radius and a height
	/// </summary>
	class CapsuleCollider final :  public ICollider {
	public:
		typedef std::shared_ptr<CapsuleCollider> Sptr;

		static CapsuleCollider::Sptr Create(float radius = 0.5f, float height = 1.0f);
		virtual ~CapsuleCollider();

		CapsuleCollider* SetRadius(float value);
		float GetRadius() const;

		CapsuleCollider* SetHeight(float value);
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

		CapsuleCollider(float radius, float height);
	};
}
