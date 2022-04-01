#include "Gameplay/Physics/PhysicsBase.h"

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

#include "Utils/GlmBulletConversions.h"
#include "Utils/ImGuiHelper.h"
#include "utils/JsonGlmHelpers.h"

namespace Gameplay::Physics {
int PhysicsBase::_editorSelectedColliderType = 0;

	PhysicsBase::PhysicsBase() : 
		IComponent(),
		_scene(nullptr),
		_colliders(std::vector<ICollider::Sptr>()),
		_shape(nullptr),
		_isShapeDirty(true),
		_collisionGroup(0x01),
		_collisionMask(0xFFFFFFFF),
		_prevScale(glm::vec3(1.0f))
	{ }

	PhysicsBase::~PhysicsBase() {
		if (_scene != nullptr) {
			delete _shape;
		}
	}

	void PhysicsBase::_RenderImGuiBase() {
		// Our colliders header
		ImGui::Separator(); ImGui::TextUnformatted("Colliders"); ImGui::Separator();
		ImGui::Indent();
		// Draw UI for all colliders
		for (int ix = 0; ix < _colliders.size(); ix++) {
			ICollider::Sptr& collider = _colliders[ix];
			ImGui::PushID(ix);
			// Draw collider type name and the delete button
			ImGui::Text((~collider->GetType()).c_str());
			ImGui::SameLine();
			if (ImGui::Button("Delete")) {
				RemoveCollider(collider);
				ix--;
				ImGui::PopID();
				continue;
			}

			collider->_isDirty |= LABEL_LEFT(ImGui::DragFloat3, "Position", &collider->_position.x, 0.01f);
			collider->_isDirty |= LABEL_LEFT(ImGui::DragFloat3, "Rotation", &collider->_rotation.x, 1.0f);
			collider->_isDirty |= LABEL_LEFT(ImGui::DragFloat3, "Scale   ", &collider->_scale.x, 0.01f);
			// Draw collider's editor
			collider->DrawImGui();

			ImGui::Separator();
			ImGui::PopID();
		}
		// Draw the add collider combo box and button
		ImGui::Combo("", &_editorSelectedColliderType, ColliderTypeComboNames);
		ImGui::SameLine();
		if (ImGui::Button("Add Collider")) {
			// Since the combo box contains all valid items (and Unknown is 0)
			// we need to add 1 to the resulting selection index
			ColliderType type = (ColliderType)(_editorSelectedColliderType + 1);
			AddCollider(ICollider::Create(type));
		}
		ImGui::Unindent();
	}


	void PhysicsBase::ToJsonBase(nlohmann::json& output) const {
		output["group"] = _collisionGroup;
		output["mask"]  = _collisionMask;
		// Make an array and store all the colliders
		output["colliders"] = std::vector<nlohmann::json>();
		for (auto& collider : _colliders) {
			nlohmann::json blob;
			blob["guid"] = collider->_guid.str();
			blob["type"] = ~collider->_type;
			blob["position"] = (collider->_position);
			blob["rotation"] = (collider->_rotation);
			blob["scale"]    = (collider->_scale);
			collider->ToJson(blob);
			output["colliders"].push_back(blob);
		}
	}

	void PhysicsBase::FromJsonBase(const nlohmann::json& input) {
		// Only the group and mask are common for all collision types
		_collisionGroup = input["group"];
		_collisionMask  = input["mask"];

		// There should always be colliders, but just to be safe...
		if (input.contains("colliders") && input["colliders"].is_array()) {
			// Iterate over all colliders
			for (auto& blob : input["colliders"]) {
				// Get the type
				ColliderType type = ParseColliderType(blob["type"], ColliderType::Unknown);
				// Get the actual collider based on the type we got from our file
				ICollider::Sptr collider = ICollider::Create(type);
				// If we got a valid shape and thus collider, load and store
				if (collider != nullptr) {
					// Copy in collider info
					collider->_guid = Guid(blob["guid"]);
					collider->_position = JsonGet(blob, "position", collider->_position);
					collider->_rotation = JsonGet(blob, "rotation", collider->_rotation);
					collider->_scale    = JsonGet(blob, "scale", collider->_scale);
					// Allow the derived loading
					collider->FromJson(blob);
					// Mark dirty and store
					collider->_isDirty = true;
					_colliders.push_back(collider);
				}
			}
		}
	}


	void PhysicsBase::SetCollisionGroup(int value) {
		_collisionGroup = 1 << value;
		_isGroupMaskDirty = true;
	}

	void PhysicsBase::SetCollisionGroupMulti(int value) {
		_collisionGroup   = value;
		_isGroupMaskDirty = true;
	}

	int PhysicsBase::GetCollisionGroup() const {
		return _collisionGroup;
	}

	void PhysicsBase::SetCollisionMask(int value) {
		_collisionMask = value;
		_isGroupMaskDirty = true;
	}

	int PhysicsBase::GetCollisionMask() const {
		return _collisionMask;
	}

	ICollider::Sptr PhysicsBase::AddCollider(const ICollider::Sptr& collider) {
		if (_scene != nullptr) {
			collider->Awake(GetGameObject());
		}
		_colliders.push_back(collider);
		_isShapeDirty = true;
		return collider;
	}

	void PhysicsBase::RemoveCollider(const ICollider::Sptr& collider) {
		auto& it = std::find(_colliders.begin(), _colliders.end(), collider);
		if (it != _colliders.end()) {
			if (collider->GetShape() != nullptr) {
				_shape->removeChildShape(collider->GetShape());
				_isShapeDirty = true;
			}
			_colliders.erase(it);
		}
	}


	void PhysicsBase::_AddColliderToShape(ICollider* collider) {
		// Create the bullet collision shape from the collider
		btCollisionShape* newShape = collider->CreateShape();
		collider->_shape = newShape;

		// If the shape actually exists
		if (newShape != nullptr) {
			// We convert our shape parameters to a bullet transform
			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(ToBt(collider->_position));
			transform.setRotation(ToBt(glm::quat(glm::radians(collider->_rotation))));
			newShape->setLocalScaling(ToBt(collider->_scale));

			// Add the shape to the compound shape
			_shape->addChildShape(transform, newShape);

			// Remove any existing collision manifolds, so that our body can properly be updated with it's new shape
			if (_scene != nullptr) {
				_scene->GetPhysicsWorld()->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(_GetBroadphaseHandle(), _scene->GetPhysicsWorld()->getDispatcher());
			}

			// Our inertia has changed, so flag mass as dirty so it's recalculated
			_isShapeDirty = true;
		}
	}

	bool PhysicsBase::_HandleShapeDirty() {
		bool wasDirty = false;
		for (auto& collider : _colliders) {
			if (collider->_isDirty) {
				// If the collider already had a shape, delete it
				if (collider->_shape != nullptr) {
					_shape->removeChildShape(collider->_shape);
					delete collider->_shape;
				}
				_AddColliderToShape(collider.get());
				collider->_isDirty = false;
				wasDirty = true;
			}
		}

		return wasDirty;
	}

	bool PhysicsBase::_HandleGroupDirty() {
		// If the group or mask have changed, notify bullet
		if (_isGroupMaskDirty) {
			_GetBroadphaseHandle()->m_collisionFilterGroup = _collisionGroup;
			_GetBroadphaseHandle()->m_collisionFilterMask  = _collisionMask;

			_isGroupMaskDirty = false;
			return true;
		}
		return false;
	}

	void PhysicsBase::_CopyGameobjectTransformTo(btTransform& transform) {

		GameObject* context = GetGameObject();

		// Copy our transform info from OpenGL
		transform.setIdentity();
		transform.setOrigin(ToBt(context->GetPosition()));	 
		transform.setRotation(ToBt(context->GetRotation()));
		if (context->GetScale() != _prevScale) {
			_shape->setLocalScaling(ToBt(context->GetScale()));
			_scene->GetPhysicsWorld()->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(_GetBroadphaseHandle(), _scene->GetPhysicsWorld()->getDispatcher());
			_prevScale = context->GetScale();
		}
	}

	void PhysicsBase::_CopyGameobjectTransformFrom(const btTransform& transform) {
		GameObject* context = GetGameObject();

		// Update the pos and rotation params
		context->SetPostion(ToGlm(transform.getOrigin()));
		context->SetRotation(ToGlm(transform.getRotation()));
	}
}
