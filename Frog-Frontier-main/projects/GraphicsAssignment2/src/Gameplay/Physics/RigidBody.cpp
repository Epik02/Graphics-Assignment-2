#include "RigidBody.h"

#include <algorithm>
#include <GLM/glm.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Utils
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/GlmBulletConversions.h"

namespace Gameplay::Physics {
	RigidBody::RigidBody(RigidBodyType type) :
		PhysicsBase(),
		_type(type),
		_mass(1.0f),
		_isMassDirty(true),
		_body(nullptr),
		_motionState(nullptr),
		_linearDamping(0.0f),
		_angularDamping(0.005f),
		_inertia(btVector3()),
		_linearVelocity(btVector3(0, 0, 0)),
		_linearVelocityDirty(false),
		_angularVelocity(btVector3(0, 0, 0)),
		_angularVelocityDirty(false),
		_angularFactor(btVector3(1,1,1)),
		_angularFactorDirty(false)
	{ }

	RigidBody::~RigidBody() {
		if (_body != nullptr) {
			// Remove from the physics world
			_scene->GetPhysicsWorld()->removeRigidBody(_body);

			// Clean up all our memory
			delete _motionState;
			delete _body;
			_colliders.clear();
		}
	}

	void RigidBody::SetMass(float value) {
		if (_type != RigidBodyType::Static) {
			_isMassDirty = value != _mass;
			_mass = value;
		}
	}

	float RigidBody::GetMass() const {
		return _type == RigidBodyType::Static ? 0.0f : _mass;
	}

	void RigidBody::SetLinearDamping(float value) {
		_linearDamping = value;
		_isDampingDirty = true;
	}

	float RigidBody::GetLinearDamping() const {
		return _linearDamping;
	}

	void RigidBody::SetAngularDamping(float value) {
		_angularDamping = value;
		_isDampingDirty = true;
	}

	float RigidBody::GetAngularDamping() const {
		return _angularDamping;
	}

	void RigidBody::SetLinearVelocity(const glm::vec3& value)
	{
		_linearVelocity = ToBt(value);
		_linearVelocityDirty = true;
	}

	const glm::vec3& RigidBody::GetLinearVelocity() const {
		return ToGlm(_linearVelocity);
	}

	void RigidBody::SetAngularVelocity(const glm::vec3& value) {
		_angularVelocity = ToBt(glm::radians(value));
		_angularVelocityDirty = true;
	}

	glm::vec3 RigidBody::GetAngularVelocity() const
	{
		return glm::degrees(ToGlm(_angularVelocity));
	}

	void RigidBody::SetAngularFactor(const glm::vec3& value) {
		_angularFactor = ToBt(value);
		_angularFactorDirty = true;
	}

	const glm::vec3& RigidBody::GetAngularFactor() const {
		return ToGlm(_angularFactor);
	}

	void RigidBody::ApplyForce(const glm::vec3& worldForce) {
		_body->applyCentralForce(ToBt(worldForce));
	}

	void RigidBody::ApplyForce(const glm::vec3& worldForce, const glm::vec3& localOffset) {
		_body->applyForce(ToBt(worldForce), ToBt(localOffset));
	}

	void RigidBody::ApplyImpulse(const glm::vec3& worldForce) {
		_body->applyCentralImpulse(ToBt(worldForce));
	}

	void RigidBody::ApplyImpulse(const glm::vec3& worldForce, const glm::vec3& localOffset) {
		_body->applyImpulse(ToBt(worldForce), ToBt(localOffset));
	}

	void RigidBody::ApplyTorque(const glm::vec3& worldTorque) {
		_body->applyTorque(ToBt(worldTorque));
	}

	void RigidBody::ApplyTorqueImpulse(const glm::vec3& worldTorque) {
		_body->applyTorqueImpulse(ToBt(worldTorque));
	}

	void RigidBody::SetType(RigidBodyType type) {
		_type = type;
		if (_body != nullptr) {
			// Remove any static or kinematic flags for the object
			int flags = _body->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT;
			flags = _body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT;

			// Set appropriate flags
			if (_type == RigidBodyType::Kinematic) {
				_body->setCollisionFlags(flags | btCollisionObject::CF_KINEMATIC_OBJECT);
				_body->setActivationState(WANTS_DEACTIVATION);
			}
			// If the object is static, disable it's gravity and notify bullet
			else if (_type == RigidBodyType::Static) {
				_body->setCollisionFlags(flags | btCollisionObject::CF_KINEMATIC_OBJECT);
				_body->setActivationState(DISABLE_DEACTIVATION);
				_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
			} else {
				// If dynamic, we need to restore gravity from the scene
				_body->setCollisionFlags(flags);
				_body->setGravity(_scene->GetPhysicsWorld()->getGravity());
				_body->setActivationState(WANTS_DEACTIVATION);
			}
		}
	}

	RigidBodyType RigidBody::GetType() const {
		return _type;
	}

	void RigidBody::PhysicsPreStep(float dt) {
		// Update any dirty state that may have changed
		_HandleStateDirty();

		if (_type != RigidBodyType::Static) {		
			btTransform transform;
			_CopyGameobjectTransformTo(transform);

			// Copy to body and to it's motion state
			if (_type == RigidBodyType::Dynamic) {
				_body->setWorldTransform(transform);
			} else {
				// Kinematics prefer to be driven my motion state for some reason :|
				_body->getMotionState()->setWorldTransform(transform); 
			}
		}
	}

	void RigidBody::PhysicsPostStep(float dt) {
		// Kinematics are driven externally and statics don't move, so only need to get data out for dynamics!
		if (_type == RigidBodyType::Dynamic && _body->isActive()) {
			btTransform transform = _body->getWorldTransform();
			_CopyGameobjectTransformFrom(transform);

			// Store a copy of our velocities
			_linearVelocity = _body->getLinearVelocity();
			_angularVelocity = _body->getAngularVelocity();
		}
	}

	void RigidBody::Awake() {
		GameObject* context = GetGameObject();
		_scene = context->GetScene();
		_prevScale = context->GetScale();

		// Awake all our colliders to let them do initialization
		// that requires the gameobject
		for (auto& collider : _colliders) {
			collider->Awake(context);
		}

		// Create our compound shape and add all colliders
		_shape = new btCompoundShape(true, _colliders.size());
		_shape->setLocalScaling(ToBt(context->GetScale()));
		for (auto& collider : _colliders) {
			_AddColliderToShape(collider.get());
		}
		_isShapeDirty = false;

		// Update inertia
		_shape->calculateLocalInertia(_mass, _inertia);
		_isMassDirty = false;

		// Create a default motion state instance for tracking the bodies motion
		_motionState = new btDefaultMotionState();

		// Get the object's starting transform, create a bullet representation for it
		btTransform transform; 
		transform.setIdentity();
		transform.setOrigin(ToBt(context->GetPosition()));
		transform.setRotation(ToBt(context->GetRotation()));
		_motionState->setWorldTransform(transform);

		// Create the bullet rigidbody and add it to the physics scene
		_body = new btRigidBody(_mass, _motionState, _shape, _inertia);
		// Add a pointer to our own weak reference to allow getting this component as a shared_ptr later
		_body->setUserPointer(&SelfRef());

		_scene->GetPhysicsWorld()->addRigidBody(_body);

		// If the object is kinematic (driven by a controller), tell bullet that
		if (_type == RigidBodyType::Kinematic) {
			_body->setCollisionFlags(_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		} 
		// If the object is static, disable it's gravity and notify bullet
		else if (_type == RigidBodyType::Static) {
			_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
			_body->setCollisionFlags(_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		}
	
		if (_type == RigidBodyType::Static) {
			_body->setActivationState(DISABLE_DEACTIVATION);
		}

		_body->setFriction(1.0f);


		// Copy over group and mask info
		_body->getBroadphaseProxy()->m_collisionFilterGroup = _collisionGroup;
		_body->getBroadphaseProxy()->m_collisionFilterMask  = _collisionMask;
	}

	void RigidBody::RenderImGui()
	{
		_isMassDirty |= LABEL_LEFT(ImGui::DragFloat, "Mass", &_mass, 0.1f, 0.0f);
		if (LABEL_LEFT(ImGui::BeginCombo, "Type", (~_type).c_str())) {
			bool selected = _type == RigidBodyType::Static;
			if (ImGui::Selectable("Static", &selected)) {
				SetType(RigidBodyType::Static);
			}
			selected = _type == RigidBodyType::Dynamic;
			if (ImGui::Selectable("Dynamic", &selected)) {
				SetType(RigidBodyType::Dynamic);
			}
			selected = _type == RigidBodyType::Kinematic;
			if (ImGui::Selectable("Kinematic", &selected)) {
				SetType(RigidBodyType::Kinematic);
			}

			ImGui::EndCombo();
		}
		_RenderImGuiBase();
	}

	nlohmann::json RigidBody::ToJson() const {
		nlohmann::json result;
		// Write out RigidBody data
		result["type"] = ~_type;
		result["mass"] = _mass;
		result["linear_damping"] = _linearDamping;
		result["angular_damping"] = _angularDamping;
		// Write out base physics data
		ToJsonBase(result);
		return result;
	}

	RigidBody::Sptr RigidBody::FromJson(const nlohmann::json& data) {
		RigidBody::Sptr result = std::make_shared<RigidBody>();
		// Read out the RigidBody config
		result->_type = ParseRigidBodyType(data["type"], RigidBodyType::Unknown);
		result->_mass = data["mass"];
		result->_linearDamping  = data["linear_damping"];
		result->_angularDamping = data["angular_damping"];
		// Read out base physics data
		result->FromJsonBase(data);
		return result;
	}

	void RigidBody::_HandleStateDirty() {
		// Only dynamic bodies have velocities
		if (_type == RigidBodyType::Dynamic) {
			// If outside code has changed our velocity, send that to Bullet
			if (_linearVelocityDirty) {
				_body->setLinearVelocity(_linearVelocity);
				_linearVelocityDirty = false;
			}

			// If outside code has changed our angular velocity, send that to Bullet
			if (_angularVelocityDirty) {
				_body->setAngularVelocity(_angularVelocity);
				_angularVelocityDirty = false;
			}

			// If outside code has changed the angular factor, send to Bullet
			if (_angularFactorDirty) {
				_body->setAngularFactor(_angularFactor);
				_angularFactorDirty = false;
			}
		}

		// If one of our colliders has changed, replace it's shape with it's
		_isMassDirty |= _HandleShapeDirty();

		// Handle updating our group or mask if they've changed
		_HandleGroupDirty();
	
		// If our damping parameters have changed, notify Bullet and clear the flag
		if (_isDampingDirty) {
			_body->setDamping(_linearDamping, _angularDamping);
			_isDampingDirty = false;
		}

		// If the mass has changed, we need to notify bullet
		if (_isMassDirty) {
			// Static bodies don't have mass or inertia
			if (_type != RigidBodyType::Static) {
				// Recalulcate our inertia properties and send to bullet
				_shape->calculateLocalInertia(_mass, _inertia);
				_body->setMassProps(_mass, _inertia);
			}
			_isMassDirty = false;
		}
	}

	btBroadphaseProxy* RigidBody::_GetBroadphaseHandle() {
		return _body != nullptr ? _body->getBroadphaseProxy() : nullptr;
	}

}

