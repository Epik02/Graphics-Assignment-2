#pragma once
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Physics/PhysicsBase.h"
#include "Gameplay/Physics/RigidBody.h"
#include "EnumToString.h"

class btPairCachingGhostObject;

namespace Gameplay::Physics {

	ENUM_FLAGS(TriggerTypeFlags, int,
		Dynamics   = 0,
		Statics    = 1,
		Kinematics = 2
	);

	/// <summary>
	/// A trigger volume defines a shape in 3D space that allows us to respond to rigid bodies
	/// entering a volume in 3D space. Handles invoking Trigger events on gameobjects
	/// </summary>
	class TriggerVolume : public PhysicsBase {
	public:
		typedef std::function<void(const std::shared_ptr<RigidBody>& obj)> TriggerCallback;

		typedef std::shared_ptr<TriggerVolume> Sptr;
		virtual ~TriggerVolume();
		TriggerVolume();

		/// <summary>
		/// Invoked for each RigidBody before the physics world is stepped forward a frame,
		/// handles body initialization, shape changes, mass changes, etc...
		/// </summary>
		/// <param name="dt">The time in seconds since the last frame</param>
		virtual void PhysicsPreStep(float dt) override;
		/// <summary>
		/// Invoked for each RigidBody after the physics world is stepped forward a frame,
		/// handles copying transform to the OpenGL state
		/// </summary>
		/// <param name="dt">The time in seconds since the last frame</param>
		virtual void PhysicsPostStep(float dt) override;

		void SetFlags(TriggerTypeFlags flags);
		TriggerTypeFlags GetFlags() const;

		// Inherited from IComponent

		virtual void Awake() override;
		virtual void RenderImGui() override;
		virtual nlohmann::json ToJson() const override;
		static TriggerVolume::Sptr FromJson(const nlohmann::json& data);
		MAKE_TYPENAME(TriggerVolume);

	protected:
		btPairCachingGhostObject*   _ghost;
		TriggerTypeFlags            _typeFlags;

		std::vector<std::weak_ptr<RigidBody>> _currentCollisions;

		virtual btBroadphaseProxy* _GetBroadphaseHandle() override;

	};
}