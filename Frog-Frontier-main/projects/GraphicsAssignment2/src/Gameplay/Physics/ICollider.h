#pragma once

#include <json.hpp>
#include <EnumToString.h>
#include <GLM/glm.hpp>

#include <btBulletCollisionCommon.h>

#include "Utils/GUID.hpp"

/// <summary>
/// Represents the shape of a collider
/// </summary>
ENUM(ColliderType, int,
	 // The collider type is not known
	 Unknown   = 0,
	 // Plane with a normal that extends to infinity
	 Plane     = 1,
	 // Simple box colliders
	 Box       = 2,
	 // Simple sphere
	 Sphere    = 3,
	 // Capsule along the Z axis
	 Capsule   = 4,
	 // Cone along the Z axis
	 Cone      = 5,
	 // Cylinder along the Z axis
	 Cylinder  = 6,
	 // Convex meshes have no inward faces, ie no caves
	 ConvexMesh = 7,
	 // Concave meshes can have inward faces (NOT IMPLEMENTED)
	 ConcaveMesh = 8,
	 // Used for creating terrain colliders,
	 // much more complex than the other colliders (NOT IMPLEMENTED)
	 Terrain   = 9
);

namespace Gameplay {
	class GameObject;
}

namespace Gameplay::Physics {
	// Stores a string that can be fed to ImGui to make a combo box
	// of all collider types
	extern const char* ColliderTypeComboNames;

	/// <summary>
	/// Base class for collider types,
	/// will be inherited from for all collider types
	/// </summary>
	class ICollider {
	public:
		typedef std::shared_ptr<ICollider> Sptr;

		virtual ~ICollider();

		/// <summary>
		/// Draws ImGui controls for this collider type. If data
		/// has been changed, make sure to mark the shape as 
		/// dirty! If a shape is dirty, it will be re-created on
		/// the next physics frame (bullet cannot re-size shapes
		/// after they have been created)
		/// </summary>
		virtual void DrawImGui() = 0;
		/// <summary>
		/// Packs information about this collider into a JSON object for
		/// serialization. Should not contain type information, that will
		/// be handled by RigidBody serialization layer
		/// </summary>
		/// <param name="blob">The JSON blob to modify and append data to</param>
		virtual void ToJson(nlohmann::json& blob) const = 0;
		/// <summary>
		/// Helper function for unpacking collider info from a JSON structure,
		/// does not need to contain information about collider type, that 
		/// will be handled by RigidBody serialization layer
		/// </summary>
		/// <param name="data">The JSON data to unpack into this instance</param>
		virtual void FromJson(const nlohmann::json& data) = 0;

		/// <summary>
		/// Allows colliders to perform initialization on object awake, 
		/// for instance to generate a mesh collider
		/// </summary>
		/// <param name="contenxt">The gameobject that this collider is an element of</param>
		virtual void Awake(GameObject* context) {};

		/// <summary>
		/// Gets the collider type of this collider instance
		/// </summary>
		virtual ColliderType GetType() const;
		/// <summary>
		/// Gets this collider's bullet collision shape
		/// </summary>
		btCollisionShape* GetShape() const;

		/// <summary>
		/// Sets the collider's position relative to it's RigidBody
		/// </summary>
		/// <param name="value">The new value for position</param>
		/// <returns>A pointer to this, should ONLY be used for operator chaining</returns>
		ICollider* SetPosition(const glm::vec3& value);
		/// <summary>
		/// Gets the collider's position relative to it's RigidBody
		/// </summary>
		const glm::vec3& GetPosition() const;

		/// <summary>
		/// Sets the collider's rotation relative to it's RigidBody
		/// </summary>
		/// <param name="value">The new value for rotation</param>
		/// <returns>A pointer to this, should ONLY be used for operator chaining</returns>
		ICollider* SetRotation(const glm::vec3& value);
		/// <summary>
		/// Gets the collider's rotation relative to it's RigidBody
		/// </summary>
		const glm::vec3& GetRotation() const;

		/// <summary>
		/// Sets the collider's scale relative to it's parent RigidBody
		/// </summary>
		/// <param name="value">The new value for scale</param>
		/// <returns>A pointer to this, should ONLY be used for operator chaining</returns>
		ICollider* SetScale(const glm::vec3& value);
		/// <summary>
		/// Gets the collider's scale relative to it's RigidBody
		/// </summary>
		const glm::vec3& GetScale() const;

		/// <summary>
		/// Gets a GUID that can be used to reference this collider later
		/// (for instance if a component wants to preserve a reference to
		/// a collider during serialization)
		/// </summary>
		Guid GetGUID() const;

		/// <summary>
		/// Helper function to create a collider based on the given
		/// collider type
		/// </summary>
		/// <param name="type">The type of collider to create</param>
		/// <returns>A new collider of the given type with default values</returns>
		static ICollider::Sptr Create(ColliderType type);

	protected:
		// Stores type 
		ColliderType _type;
		// Stores shape, note that mutable lets us modify in const functions
		mutable btCollisionShape* _shape;
		mutable bool _isDirty;

		ICollider(ColliderType type);

		/// <summary>
		/// Creates the bullet collision shape from this collider's info
		/// </summary>
		/// <returns>A btCollisionShape allocated with new</returns>
		virtual btCollisionShape* CreateShape() const = 0;

	private:
		// Allow RigidBody to access protected and private members
		friend class PhysicsBase;

		// These are private so derived classes don't accidentally use these
		glm::vec3 _position;
		glm::vec3 _rotation;
		glm::vec3 _scale;
		Guid      _guid;
	};
}