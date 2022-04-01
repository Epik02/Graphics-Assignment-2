#pragma once

#include "Gameplay/Physics/ICollider.h"

namespace Gameplay::Physics {
	/// <summary>
	/// A complex collider type that allows us to construct collision hulls from arbitrary convex meshes
	/// </summary>
	class ConvexMeshCollider final : public ICollider {
	public:
		typedef std::shared_ptr<ConvexMeshCollider> Sptr;
		static ConvexMeshCollider::Sptr Create();
		virtual ~ConvexMeshCollider();

		// Inherited from ICollider
		virtual void Awake(GameObject* context) override;
		virtual void DrawImGui() override;
		virtual void ToJson(nlohmann::json& blob) const override;
		virtual void FromJson(const nlohmann::json& data) override;

	protected:
		btTriangleMesh* _triMesh;
		ConvexMeshCollider();

		virtual btCollisionShape* CreateShape() const override;
	};
}