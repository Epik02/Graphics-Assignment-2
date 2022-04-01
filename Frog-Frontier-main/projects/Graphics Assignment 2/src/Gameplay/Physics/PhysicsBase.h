#pragma once
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Physics/ICollider.h"

class btTransform;

namespace Gameplay {
	class Scene;

	namespace Physics {
		/// <summary>
		/// Provides a base class for physics components, including shape generation and utilities
		/// for converting to and from Bullet transforms
		/// </summary>
		class PhysicsBase : public IComponent {
		public:
			typedef std::shared_ptr<PhysicsBase> Sptr;
			virtual ~PhysicsBase();

			/// <summary>
			/// A value between 0 and 31
			/// 
			/// Sets the collision group for the body (using the formula 1 << value)
			/// 
			/// When bullet goes to resolve collisions, only objects who's groups and masks 
			/// bitwise and (a & b) into a non-zero value are considered for collision. 
			/// Combined with CollisionMask, this allows objects to only collide with certain 
			/// other objects. For instance, if you have enemies that you do not want to be able
			/// to collide, you would assign them to some group (ex enemies are group 3), and
			/// then set their mask to a value where the corresponding bit is 0 (ex: 0b11110111)
			/// </summary>
			/// <param name="value">The new collision group for the object</param>
			void SetCollisionGroup(int value);
			/// <summary>
			/// Sets this object to belong to multiple collision groups, value should
			/// be a bitwise or (a | b) of all the groups that the object should belong
			/// to. See SetCollisionGroup
			/// </summary>
			/// <param name="value">The new muli-group value for collisiong group</param>
			void SetCollisionGroupMulti(int value);
			/// <summary>
			/// Gets the collision group for this object
			/// </summary>
			int GetCollisionGroup() const;

			/// <summary>
			/// Sets the mask of which collision groups this object should collide
			/// with. This is a bitwise mask, and should align with the collision
			/// groups
			/// </summary>
			/// <param name="value">The new collision mask for the object</param>
			void SetCollisionMask(int value);
			/// <summary>
			/// Gets the collision mask for this object
			/// </summary>
			int GetCollisionMask() const;

			/// <summary>
			/// Adds a new collider to this rigidbody.
			/// Multiple colliders can be added to a rigidbody, as internally it
			/// uses a compound shape collider
			/// </summary>
			/// <param name="collider">The collider to add to this body</param>
			/// <returns></returns>
			virtual ICollider::Sptr AddCollider(const ICollider::Sptr& collider);
			/// <summary>
			/// Removes a collider from this RigidBody
			/// </summary>
			/// <param name="collider">The collider to remove</param>
			void RemoveCollider(const ICollider::Sptr& collider);


			/// <summary>
			/// Invoked for each RigidBody before the physics world is stepped forward a frame,
			/// handles body initialization, shape changes, mass changes, etc...
			/// </summary>
			/// <param name="dt">The time in seconds since the last frame</param>
			virtual void PhysicsPreStep(float dt) = 0;
			/// <summary>
			/// Invoked for each RigidBody after the physics world is stepped forward a frame,
			/// handles copying transform to the OpenGL state
			/// </summary>
			/// <param name="dt">The time in seconds since the last frame</param>
			virtual void PhysicsPostStep(float dt) = 0;

			// Delete awake to ensure derived classes override it

			virtual void Awake() = 0;
		protected:
			Scene*        _scene;

			// Stores the bullet shape associated with the physics object
			btCompoundShape* _shape;

			// List of colliders and whether they have been changed
			std::vector<ICollider::Sptr> _colliders;
			mutable bool  _isShapeDirty;

			// This lets us have objects that do not collide with each other!
			int _collisionGroup;
			int _collisionMask;
			mutable bool _isGroupMaskDirty;

			glm::vec3 _prevScale;

			PhysicsBase();

			void _RenderImGuiBase();

			void ToJsonBase(nlohmann::json& output) const;
			void FromJsonBase(const nlohmann::json& input);

			// Handles adding a collider to our compound shape
			void _AddColliderToShape(ICollider* collider);

			// Handles resolving any dirty state stuff for our object
			bool _HandleShapeDirty();

			bool _HandleGroupDirty();

			// Copies the gameobject's transform the the bullet transform
			void _CopyGameobjectTransformTo(btTransform& transform);
			void _CopyGameobjectTransformFrom(const btTransform& transform);

			// Gets the bullet broadphase proxy that we can use for clearing collisions
			virtual btBroadphaseProxy* _GetBroadphaseHandle() = 0;

			static int _editorSelectedColliderType;
		};
	}
}