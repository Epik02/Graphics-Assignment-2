#pragma once
#include <EnumToString.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Physics/ICollider.h"
#include "Gameplay/Physics/PhysicsBase.h"

ENUM(RigidBodyType, int,
	Unknown   = 0,
	// Does not move within the scene, EX: level geometry
    Static    = 1,
	// Objects that are driven by physics
	Dynamic   = 2,
	// Objects that are driven by some control method, ex: doors, moving
	// platforms, etc...
	// kinematic objects will not collide with static or other
	// kinematic objects
	Kinematic = 3,
);

// We'll need to get stuff from the scene, which we can grab from our parent GO
namespace Gameplay { class Scene; }

namespace Gameplay::Physics {
	/// <summary>
	/// A rigid body is a static, kinematic, or dynamic body that represents a collision object
	/// within our physics scene
	/// </summary>
	class RigidBody : public PhysicsBase {
	public:
		typedef std::shared_ptr<RigidBody> Sptr;

		RigidBody(RigidBodyType type = RigidBodyType::Static);
		virtual ~RigidBody();

		/// <summary>
		/// Sets the mass for this object in KG
		/// </summary>
		/// <param name="value">The new mass of the object</param>
		void SetMass(float value);
		/// <summary>
		/// Gets the object's mass in KG
		/// </summary>
		float GetMass() const;

		/// <summary>
		/// Sets the linear damping for this object
		/// This is how quickly the object will bleed off linear (moving)
		/// velocity without any outside forces acting on it. Can be
		/// thought of as "air drag" for the object
		/// </summary>
		/// <param name="value">he new value for linear drag, default 0</param>
		void SetLinearDamping(float value);
		/// <summary>
		/// Gets the linear damping (ie drag) for this object
		/// </summary>
		float GetLinearDamping() const;

		/// <summary>
		/// Sets the angular damping for this object
		/// This is how quickly the object will bleed off angular (rotational)
		/// velocity without any outside forces acting on it. Can be
		/// thought of as "air drag" for the object
		/// </summary>
		/// <param name="value">The new value for angular drag, default 0.005f</param>
		void SetAngularDamping(float value);
		/// <summary>
		/// Gets the angular damping (ie drag) for this object
		/// </summary>
		float GetAngularDamping() const;

		/// <summary>
		/// Sets the linear velocity for this body. If called before Awake,
		/// will set the body's initial velocity
		/// </summary>
		/// <param name="value">The velocity per axis in m/s</param>
		void SetLinearVelocity(const glm::vec3& value);
		/// <summary>
		/// Gets the bodies linear velocity, in m/s
		/// </summary>
		const glm::vec3& GetLinearVelocity() const;

		/// <summary>
		/// Sets the angular velocity for this body. If called before Awake,
		/// will set the body's angular velocity
		/// </summary>
		/// <param name="value">The velocity per axis in degrees/s</param>
		void SetAngularVelocity(const glm::vec3& value);
		/// <summary>
		/// Gets the bodies angular velocity, in degrees/s
		/// </summary>
		glm::vec3 GetAngularVelocity() const;

		/// <summary>
		/// Sets the angular factor for this body. The velocity along each
		/// axis will be scaled by this amount. For instance, to prevent a
		/// body from rotating along the Z axis, you could set the angular
		/// factor to (1, 1, 0)
		/// </summary>
		/// <param name="value">The angular factor per axis</param>
		void SetAngularFactor(const glm::vec3& value);
		/// <summary>
		/// Gets the bodies angular factor
		/// </summary>
		const glm::vec3& GetAngularFactor() const;

		/// <summary>
		/// Applies a force in world space to this object, this would be used
		/// if you want to apply a force every frame on an object
		/// </summary>
		/// <param name="worldForce">The force in world space and Newtons</param>
		void ApplyForce(const glm::vec3& worldForce);
		/// <summary>
		/// Applies a force in world space to this object, this would be used
		/// if you want to apply a force every frame on an object
		/// </summary>
		/// <param name="worldForce">The force in world space and Newtons</param>
		/// <param name="localOffset">The offset from the object in worldspace units to apply the force</param>
		void ApplyForce(const glm::vec3& worldForce, const glm::vec3& localOffset);
		/// <summary>
		/// Applies a direct change in velocity on the object
		/// </summary>
		/// <param name="worldForce">The force in world space and m/s</param>
		void ApplyImpulse(const glm::vec3& worldForce);
		/// <summary>
		/// Applies a direct change in velocity on the object, relative to a given
		/// offset to the object
		/// </summary>
		/// <param name="worldForce">The force in world space and m/s</param>
		/// <param name="localOffset">The offset from the object in worldspace units to apply the impulse</param>
		void ApplyImpulse(const glm::vec3& worldForce, const glm::vec3& localOffset);
		/// <summary>
		/// Applies torque (rotational energy) to the object
		/// </summary>
		/// <param name="worldTorque">The torque, in world units and in Nm</param>
		void ApplyTorque(const glm::vec3& worldTorque);
		/// <summary>
		/// Applies an immediate change in angular momentum to an object
		/// </summary>
		/// <param name="worldTorque">The torque, in radians per second and world space</param>
		void ApplyTorqueImpulse(const glm::vec3& worldTorque);

		/// <summary>
		/// Sets the type of rigid body (static, dynamic, kinematic)
		/// </summary>
		/// <param name="type">The new type for the body</param>
		void SetType(RigidBodyType type);
		/// <summary>
		/// Returns this RigidBody's type (static, dynamic, kinematic)
		/// </summary>
		RigidBodyType GetType() const;

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

		// Inherited from IComponent
		virtual void Awake() override;
		virtual void RenderImGui() override;
		virtual nlohmann::json ToJson() const override;
		static RigidBody::Sptr FromJson(const nlohmann::json& data);
		MAKE_TYPENAME(RigidBody)


	protected:
		// The physics update mode for the body (static, dynamic, kinematic)
		RigidBodyType _type;

		// The mass of the object, in KG
		float         _mass;
		mutable bool  _isMassDirty;

		// Controls phsyics damping (how quickly motion stops)
		// can be thought of as the "air resistance"
		float _angularDamping;
		float _linearDamping;
		mutable bool _isDampingDirty;

		// Our bullet state stuff
		btRigidBody*     _body;
		btMotionState*   _motionState;
		btVector3        _inertia;
		btVector3        _linearVelocity;
		bool             _linearVelocityDirty;
		btVector3        _angularVelocity;
		bool             _angularVelocityDirty;
		btVector3        _angularFactor;
		bool             _angularFactorDirty;

		// Handles resolving any dirty state stuff for our object
		void _HandleStateDirty();

		virtual btBroadphaseProxy* _GetBroadphaseHandle() override;
	};
}