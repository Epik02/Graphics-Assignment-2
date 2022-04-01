#include "Gameplay/Physics/TriggerVolume.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "Utils/GlmBulletConversions.h"

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"


namespace Gameplay::Physics {
	TriggerVolume::TriggerVolume() :
		PhysicsBase(),
		_ghost(nullptr),
		_typeFlags(TriggerTypeFlags::Dynamics)
	{
	}

	TriggerVolume::~TriggerVolume() {
		if (_ghost != nullptr) {
			_scene->GetPhysicsWorld()->removeCollisionObject(_ghost);
			delete _ghost;
		}
	}

	void TriggerVolume::PhysicsPreStep(float dt) {
		// Update any dirty state that may have changed
		_HandleShapeDirty();
		_HandleGroupDirty();

		// Copy our transform info from OpenGL
		GameObject* context = GetGameObject();
		btTransform transform;
		_CopyGameobjectTransformTo(transform);

		_ghost->setWorldTransform(transform);
	}

	void TriggerVolume::PhysicsPostStep(float dt) {
		// This will store all the objects inside the trigger this frame
		std::vector<std::weak_ptr<RigidBody>> thisFrameCollision;

		// Get all our collisions from from the world
		_scene->GetPhysicsWorld()->getDispatcher()->dispatchAllCollisionPairs(_ghost->getOverlappingPairCache(), _scene->GetPhysicsWorld()->getDispatchInfo(), _scene->GetPhysicsWorld()->getDispatcher());
		btBroadphasePairArray& collisionPairs = _ghost->getOverlappingPairCache()->getOverlappingPairArray();

		// Determine how many objects are intersecting the volume
		const int numObjects=collisionPairs.size();
		thisFrameCollision.reserve(numObjects);

		// Will store our contact manifolds, can be static to be shared between frames and instances
		static btManifoldArray	m_manifoldArray;

		// Iterate over all the objects that we're colliding with
		for (int i = 0; i < numObjects; ++i) {
			// resize the number of manifolds to zero for everything we collide with
			m_manifoldArray.resize(0);

			// Get the btCollisionObject that we're colliding with
			btCollisionObject *obj = _ghost->getOverlappingObject(i);

			// Get the contact pair and resolve contact manifolds
			btBroadphasePair* pair = &collisionPairs[i];
			if (pair != nullptr && pair->m_algorithm != nullptr) {
				pair->m_algorithm->getAllContactManifolds(m_manifoldArray);
			} else {
				continue;
			}

			// Iterate over all the contact manifolds and check if any of them have contacts
			bool hasCollision = false;
			for (int j=0; j < m_manifoldArray.size(); j++) {
				btPersistentManifold* manifold = m_manifoldArray[j];
				if (manifold != nullptr && manifold->getNumContacts() > 0) {
					hasCollision = true;
					break;
				}
			}

			// If we have contacts and the object's group matches our mask (since this isn't filtered for us)
			if (hasCollision && (obj->getBroadphaseHandle()->m_collisionFilterGroup & _collisionMask)) {
				// Make sure the internal type is a bullet rigid body (no trigger-trigger interactions)
				if (obj->getInternalType() == btCollisionObject::CO_RIGID_BODY) {
					// Get the collision object as a btRigidBody
					const btRigidBody *body = (const btRigidBody *)obj;

					// Make sure that the object is not a kinematic or static object (note: you may want
					// to modify this behaviour depending on your game)
					if (((body->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT & btCollisionObject::CF_KINEMATIC_OBJECT) == 0) ||
						((body->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) == *(_typeFlags & TriggerTypeFlags::Statics)) ||
						((body->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT) == *(_typeFlags & TriggerTypeFlags::Kinematics))) {

						// Extract the weak pointer that we stored in all our rigidbody user pointers
						std::weak_ptr<IComponent> rawPtr = *reinterpret_cast<std::weak_ptr<IComponent>*>(body->getUserPointer());
						// Cast lock the raw pointer and cast up to a RigidBody
						std::shared_ptr<RigidBody> physicsPtr = std::dynamic_pointer_cast<RigidBody>(rawPtr.lock());

						// As long as we got a pointer out, we can proceed to try and invoke
						if (physicsPtr != nullptr && physicsPtr->GetGameObject() != GetGameObject()) {
							// Add the object to the known collisions for this frame
							thisFrameCollision.push_back(physicsPtr);

							// Check to see if the object has been added to our object cache
							auto& it = std::find_if(_currentCollisions.begin(), _currentCollisions.end(), [&](const std::weak_ptr<RigidBody>& item) {
								return item.lock() == physicsPtr;
							});

							// If the object is NOT in the cache, we invoke all the callbacks
							if (it == _currentCollisions.end()) {
								physicsPtr->GetGameObject()->OnEnteredTrigger(std::dynamic_pointer_cast<TriggerVolume>(SelfRef().lock()));
								GetGameObject()->OnTriggerVolumeEntered(physicsPtr);
							}
						}
					}
				}

			}
		}
	
		// Compare our current frame list to the previous frame to see if anything has left
		for (auto& weakPtr : _currentCollisions) {
			// Search the the current list to see if the item still exists
			auto& it = std::find_if(thisFrameCollision.begin(), thisFrameCollision.end(), [&](const std::weak_ptr<RigidBody>& item) {
				return item.lock() == weakPtr.lock();
			});

			// If the item no longer exists in the list, we need to invoke exit callbacks
			if (it == thisFrameCollision.end()) {
				RigidBody::Sptr body = weakPtr.lock();
				if (body != nullptr) {
					body->GetGameObject()->OnLeavingTrigger(std::dynamic_pointer_cast<TriggerVolume>(SelfRef().lock()));
					GetGameObject()->OnTriggerVolumeLeaving(body);
				}
			}
		}

		// Load the contents of the current collision items into the cache
		_currentCollisions.swap(thisFrameCollision);
	}

	void TriggerVolume::Awake() {
		GameObject* context = GetGameObject();
		_scene = GetGameObject()->GetScene();
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

		// Create the ghost object
		_ghost = new btPairCachingGhostObject();
		_ghost->setCollisionShape(_shape);
		_ghost->setUserPointer(&SelfRef());
		_ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

		// Get the transform and send it to the ghost
		btTransform transform;
		_CopyGameobjectTransformTo(transform);
		_ghost->setWorldTransform(transform);

		// Add the object to the scene
		_scene->GetPhysicsWorld()->addCollisionObject(_ghost);
		
		// Copy over group and mask info
		_ghost->getBroadphaseHandle()->m_collisionFilterGroup = _collisionGroup;
		_ghost->getBroadphaseHandle()->m_collisionFilterMask  = _collisionMask;
	}

	void TriggerVolume::RenderImGui() {
		_RenderImGuiBase();
	}

	nlohmann::json TriggerVolume::ToJson() const {
		nlohmann::json result;
		ToJsonBase(result);
		return result;
	}

	TriggerVolume::Sptr TriggerVolume::FromJson(const nlohmann::json& data) {
		TriggerVolume::Sptr result = std::make_shared<TriggerVolume>();
		result->FromJsonBase(data);
		return result;
	}

	btBroadphaseProxy* TriggerVolume::_GetBroadphaseHandle() {
		return _ghost != nullptr ? _ghost->getBroadphaseHandle() : nullptr;
	}

	void TriggerVolume::SetFlags(TriggerTypeFlags flags) {
		_typeFlags = flags;
	}

	Gameplay::Physics::TriggerTypeFlags TriggerVolume::GetFlags() const {
		return _typeFlags;
	}

}

