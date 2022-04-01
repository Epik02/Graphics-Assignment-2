#include "GameObject.h"

// Utilities
#include "Utils/JsonGlmHelpers.h"

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/quaternion.hpp"
#include "GLM/glm.hpp"
#include "Utils/GlmDefines.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/Scene.h"

namespace Gameplay {
	GameObject::GameObject() :
		IResource(),
		Name("Unknown"),
		HideInHierarchy(false),
		_components(std::vector<IComponent::Sptr>()),
		_scene(nullptr),
		_position(ZERO),
		_rotation(glm::quat(glm::vec3(0.0f))),
		_scale(ONE),
		_localTransform(MAT4_IDENTITY),
		_inverseLocalTransform(MAT4_IDENTITY),
		_isLocalTransformDirty(true),
		_worldTransform(MAT4_IDENTITY),
		_inverseWorldTransform(MAT4_IDENTITY),
		_isWorldTransformDirty(true),
		_parent(WeakRef()),
		_children(std::vector<WeakRef>())
	{ }

	void GameObject::_RecalcLocalTransform() const
	{
		if (_isLocalTransformDirty) {
			_localTransform = glm::translate(MAT4_IDENTITY, _position) * glm::mat4_cast(_rotation) * glm::scale(MAT4_IDENTITY, _scale);
			_inverseLocalTransform = glm::inverse(_localTransform);
			_isLocalTransformDirty = false;
			_isWorldTransformDirty = true;

			// Dirty all the child objects world transforms
			for (const auto& childPtr : _children) {
				GameObject::Sptr childSptr = childPtr;
				if (childSptr != nullptr) {
					childSptr->_isWorldTransformDirty = true;
				}
			}
		}
	}

	void GameObject::_RecalcWorldTransform() const {
		// Start by determining our local transform if required
		_RecalcLocalTransform();

		// If our world transform has been marked as dirty, we need to recalculate it!
		if (_isWorldTransformDirty) {
			GameObject::Sptr parent = _parent;

			// If out parent exists, we apply our local transformation relative to the parent's world transformation
			if (parent != nullptr) {
				_worldTransform = parent->GetTransform() * _localTransform;
				_inverseWorldTransform = glm::inverse(_worldTransform);
			}

			// If our parent is null, we can simply use the local transform as the world transform
			else {
				_worldTransform = _localTransform;
				_inverseWorldTransform = _inverseLocalTransform;
			}
			_isWorldTransformDirty = false;
		}
	}

	void GameObject::_PurgeDeletedChildren() {
		auto it = std::remove_if(_children.begin(), _children.end(), [](WeakRef child) { 
			return child == nullptr; 
		});
		_children.erase(it, _children.end());
	}

	void GameObject::LookAt(const glm::vec3& point) {
		glm::mat4 rot = glm::lookAt(_position, point, glm::vec3(0.0f, 0.0f, 1.0f));
		// Take the conjugate of the quaternion, as lookAt returns the *inverse* rotation
		SetRotation(glm::conjugate(glm::quat_cast(rot)));
	}

	void GameObject::OnEnteredTrigger(const std::shared_ptr<Physics::TriggerVolume>& trigger) {
		for (auto& component : _components) {
			component->OnEnteredTrigger(trigger);
		}
	}

	void GameObject::OnLeavingTrigger(const std::shared_ptr<Physics::TriggerVolume>& trigger) {
		for (auto& component : _components) {
			component->OnLeavingTrigger(trigger);
		}
	}

	void GameObject::OnTriggerVolumeEntered(const std::shared_ptr<Physics::RigidBody>& trigger) {
		for (auto& component : _components) {
			component->OnTriggerVolumeEntered(trigger);
		}
	}

	void GameObject::OnTriggerVolumeLeaving(const std::shared_ptr<Physics::RigidBody>& trigger) {
		for (auto& component : _components) {
			component->OnTriggerVolumeLeaving(trigger);
		}
	}

	void GameObject::SetPostion(const glm::vec3& position) {
		_position = position;
		_isLocalTransformDirty = true;
	}

	const glm::vec3& GameObject::GetPosition() const {
		return _position;
	}

	glm::vec3 GameObject::GetWorldPosition() const {
		return this->GetTransform()[3];
	}

	void GameObject::SetRotation(const glm::quat& value) {
		_rotation = value;
		_isLocalTransformDirty = true;
	}

	const glm::quat& GameObject::GetRotation() const {
		return _rotation;
	}

	void GameObject::SetRotation(const glm::vec3& eulerAngles) {
		_rotation = glm::quat(glm::radians(eulerAngles));
		_isLocalTransformDirty = true;
	}

	glm::vec3 GameObject::GetRotationEuler() const {
		return glm::degrees(glm::eulerAngles(_rotation));
	}

	void GameObject::SetScale(const glm::vec3& value) {
		_scale = value;
		_isLocalTransformDirty = true;
	}

	const glm::vec3& GameObject::GetScale() const {
		return _scale;
	}

	const glm::mat4& GameObject::GetTransform() const {
		_RecalcWorldTransform();
		return _worldTransform;
	}

	const glm::mat4& GameObject::GetInverseTransform() const {
		_RecalcWorldTransform();
		return _inverseWorldTransform;
	}

	const glm::mat4& GameObject::GetLocalTransform() const
	{
		_RecalcLocalTransform();
		return _localTransform;
	}

	const glm::mat4& GameObject::GetInverseLocalTransform() const {
		_RecalcLocalTransform();
		return _inverseLocalTransform;
	}

	void GameObject::RenderGUI() {
		// Prune children
		auto it = std::remove_if(_children.begin(), _children.end(), [](const WeakRef& child) { return !child.IsAlive(); });
		if (it != _children.end()) {
			_children.erase(it);
		}

		for (auto& component : _components) {
			if (component->IsEnabled) {
				component->StartGUI();
			}
		}
		for (auto& component : _components) {
			if (component->IsEnabled) {
				component->RenderGUI();
			}
		}
		for (auto& child : _children) {
			child->RenderGUI();
		}
		for (auto& component : _components) {
			if (component->IsEnabled) {
				component->FinishGUI();
			}
		}
	}

	Scene* GameObject::GetScene() const {
		return _scene;
	}

	void GameObject::Awake() {
		for (auto& component : _components) {
			component->Awake();
		}
	}

	void GameObject::Update(float dt) {
		for (auto& component : _components) {
			if (component->IsEnabled) {
				component->Update(dt);
			}
		}

		_RecalcLocalTransform();
		_RecalcWorldTransform();
		_PurgeDeletedChildren();
	}

	bool GameObject::Has(const std::type_index& type) {
		// Iterate over all the pointers in the components list
		for (const auto& ptr : _components) {
			// If the pointer type matches T, we return true
			if (std::type_index(typeid(*ptr.get())) == type) {
				return true;
			}
		}
		return false;
	}

	std::shared_ptr<IComponent> GameObject::Get(const std::type_index& type)
	{
		// Iterate over all the pointers in the binding list
		for (const auto& ptr : _components) {
			// If the pointer type matches T, we return that behaviour, making sure to cast it back to the requested type
			if (std::type_index(typeid(*ptr.get())) == type) {
				return ptr;
			}
		}
		return nullptr;
	}

	std::shared_ptr<IComponent> GameObject::Add(const std::type_index& type)
	{
		LOG_ASSERT(!Has(type), "Cannot add 2 instances of a component type to a game object");

		// Make a new component, forwarding the arguments
		std::shared_ptr<IComponent> component = _scene->_components.Create(type);
		// Let the component know we are the parent
		component->_context = this;

		// Append it to the binding component's storage, and invoke the OnLoad
		_components.push_back(component);
		component->OnLoad();

		if (_scene->GetIsAwake()) {
			component->Awake();
		}

		return component;
	}

	void GameObject::AddChild(const GameObject::Sptr& child) {
		// If the object already has a parent, remove it from the other object
		if (child->_parent != nullptr) {
			child->_parent->RemoveChild(child);
		}

		// Make sure the object isn't already a child of this object
		auto it = std::find_if(_children.begin(), _children.end(), [child](GameObject::WeakRef wPtr) { return wPtr == child;});

		// As long as the child is not already a child of this gameobject, add it
		if (it == _children.end()) {
			// Add child, set parent, and mark it's world transform as dirty, since the parent's transform now 
			// applies to the child
			_children.push_back(child);
			child->_parent = _selfRef.lock();
			child->_isWorldTransformDirty = true;
		} else {
			LOG_WARN("Attempting to add same child twice, ignoring: {}", child->Name);
		}
	}

	bool GameObject::RemoveChild(const GameObject::Sptr& child) {
		// Find the child in our list of children if it exists
		auto it = std::find_if(_children.begin(), _children.end(), [child](GameObject::WeakRef wPtr) { return wPtr == child; });
		
		if (it != _children.end()) { 
			// Clear the object's parent and remove from our list of children
			child->_parent.Reset();
			_children.erase(it);
			return true;
		} else {
			return false;
		}
	}

	const std::vector<GameObject::WeakRef>& GameObject::GetChildren() const {
		return _children;
	}

	GameObject::Sptr GameObject::GetParent() const {
		return _parent;
	}

	void GameObject::DrawImGui(bool invokedFromScene) {
		if (invokedFromScene && _parent != nullptr) {
			return;
		}

		ImGui::PushID(this); // Push a new ImGui ID scope for this object
		// Since we're allowing names to change, we need to use the ### to have a static ID for the header
		static char buffer[256];
		sprintf_s(buffer, 256, "%s###GO_HEADER", Name.c_str());
		if (ImGui::CollapsingHeader(buffer)) {
			ImGui::Indent();

			// Draw a textbox for our name
			static char nameBuff[256];
			memcpy(nameBuff, Name.c_str(), Name.size());
			nameBuff[Name.size()] = '\0';
			if (ImGui::InputText("", nameBuff, 256)) {
				Name = nameBuff;
			}
			ImGui::SameLine();
			if (ImGuiHelper::WarningButton("Delete")) {
				ImGui::OpenPopup("Delete GameObject");
			}

			// Draw our delete modal
			if (ImGui::BeginPopupModal("Delete GameObject")) {
				ImGui::Text("Are you sure you want to delete this game object?");
				if (ImGuiHelper::WarningButton("Yes")) {
					// Remove ourselves from the scene
					_scene->RemoveGameObject(SelfRef());

					// Restore imgui state so we can early bail
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::Unindent();
					ImGui::PopID();
					return;
				}
				ImGui::SameLine();
				if (ImGui::Button("No")) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			// Render position label
			_isLocalTransformDirty |= LABEL_LEFT(ImGui::DragFloat3, "Position", &_position.x, 0.01f);
			
			// Get the ImGui storage state so we can avoid gimbal locking issues by storing euler angles in the editor
			glm::vec3 euler = GetRotationEuler();
			ImGuiStorage* guiStore = ImGui::GetStateStorage();

			// Extract the angles from the storage, note that we're only using the address of the position for unique IDs
			euler.x = guiStore->GetFloat(ImGui::GetID(&_position.x), euler.x);
			euler.y = guiStore->GetFloat(ImGui::GetID(&_position.y), euler.y);
			euler.z = guiStore->GetFloat(ImGui::GetID(&_position.z), euler.z);

			//Draw the slider for angles
			if (LABEL_LEFT(ImGui::DragFloat3, "Rotation", &euler.x, 1.0f)) {
				// Wrap to the -180.0f to 180.0f range for safety
				euler = Wrap(euler, -180.0f, 180.0f);

				// Update the editor state with our new values
				guiStore->SetFloat(ImGui::GetID(&_position.x), euler.x);
				guiStore->SetFloat(ImGui::GetID(&_position.y), euler.y);
				guiStore->SetFloat(ImGui::GetID(&_position.z), euler.z);

				//Send new rotation to the gameobject
				SetRotation(euler);
			}
			
			// Draw the scale
			_isLocalTransformDirty |= LABEL_LEFT(ImGui::DragFloat3, "Scale   ", &_scale.x, 0.01f, 0.0f);

			ImGui::Separator();
			ImGui::TextUnformatted("Components");
			ImGui::Separator();

			// Render each component under it's own header
			for (int ix = 0; ix < _components.size(); ix++) {
				std::shared_ptr<IComponent> component = _components[ix];
				if (ImGui::CollapsingHeader(component->ComponentTypeName().c_str())) {
					ImGui::PushID(component.get()); 
					component->RenderImGui();
					// Render a delete button for the component
					if (ImGuiHelper::WarningButton("Delete")) {
						_components.erase(_components.begin() + ix);
						ix--;
					}
					ImGui::PopID();
				}
			}
			ImGui::Separator();

			// Render a combo box for selecting a component to add
			static std::string preview = "";
			static std::optional<std::type_index> selectedType;
			if (ImGui::BeginCombo("##AddComponents", preview.c_str())) {
				_scene->Components().EachType([&](const std::string& typeName, const std::type_index type) {
					// Hide component types already added
					if (!Has(type)) {
						bool isSelected = typeName == preview;
						if (ImGui::Selectable(typeName.c_str(), &isSelected)) {
							preview = typeName;
							selectedType = type;
						}
					}
				});
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			// Button to add component and reset the selected type
			if (ImGui::Button("Add Component") && selectedType.has_value() && !Has(selectedType.value())) {
				Add(selectedType.value());
				selectedType.reset();
				preview = "";
			}

			ImGui::Separator();
			ImGui::TextUnformatted("Children");
			ImGui::Separator();

			_PurgeDeletedChildren();
			for (auto& child : _children) {
				child->DrawImGui(false);
			}

			ImGui::Unindent();
		}
		ImGui::PopID(); // Pop the ImGui ID scope for the object

		// For if we're not in play mode
		_RecalcLocalTransform();
		_RecalcWorldTransform();
	}

	std::shared_ptr<GameObject> GameObject::SelfRef() {
		return _selfRef.lock();
	}

	GameObject::Sptr GameObject::FromJson(Scene* scene, const nlohmann::json& data)
	{
		// We need to manually construct since the GameObject constructor is
		// protected. We can call it here since Scene is a friend class of GameObjects
		GameObject::Sptr result(new GameObject());
		result->_scene = scene;

		// Load in basic info
		result->Name = data["name"];
		result->_guid = Guid(data["guid"]);
		result->_parent = WeakRef(Guid(data.contains("parent") ? data["parent"] : "null"), nullptr);
		result->_position = (data["position"]);
		result->_rotation = (data["rotation"]);
		result->_scale    = (data["scale"]);
		result->HideInHierarchy = JsonGet(data, "hide_in_inspector", false);
		result->_isLocalTransformDirty = true;
		result->_isWorldTransformDirty = true;

		// Since our components are stored based on the type name, we iterate
		// on the keys and values from the components object
		nlohmann::json components = data["components"];
		for (auto& [typeName, value] : components.items()) {
			// We need to reference the component registry to load our components
			// based on the type name (note that all component types need to be
			// registered at the start of the application)
			IComponent::Sptr component = scene->Components().Load(typeName, value);
			component->_context = result.get();

			// Add component to object and allow it to perform self initialization
			result->_components.push_back(component);
			component->OnLoad();
		}

		return result;
	}

	nlohmann::json GameObject::ToJson() const {
		GameObject::Sptr parent = _parent;
		nlohmann::json result = {
			{ "name", Name },
			{ "guid", _guid.str() },
			{ "position", _position },
			{ "rotation", _rotation },
			{ "scale",    _scale },
			{ "parent",   parent == nullptr ? "null" : parent->_guid.str() },
			{ "hide_in_inspector", HideInHierarchy }
		};
		result["components"] = nlohmann::json();
		for (auto& component : _components) {
			result["components"][component->ComponentTypeName()] = component->ToJson();
			IComponent::SaveBaseJson(component, result["components"][component->ComponentTypeName()]);
		}
		result["children"] = std::vector<nlohmann::json>();
		for (auto& child : _children) {
			GameObject::Sptr childPtr = child;
			if (childPtr != nullptr) {
				result["children"].push_back(childPtr->ToJson());
			}
		}
		return result;
	}

	Gameplay::GameObject::WeakRef& GameObject::WeakRef::operator=(const GameObject::Sptr& ptr) {
		ResourceGUID = ptr->GetGUID();
		SceneContext = ptr->GetScene();
		Ptr = ptr;
		isNull = ptr == nullptr;
		return *this;
	}

	GameObject::WeakRef::WeakRef(const GameObject::Sptr& ptr) {
		*this = ptr;
		isNull = ptr == nullptr;
	}

	GameObject::WeakRef::WeakRef() :
		ResourceGUID(Guid()),
		SceneContext(nullptr),
		Ptr(std::weak_ptr<GameObject>()),
		isNull(true)
	{ }

	GameObject::WeakRef::WeakRef(const Guid& guid, const Scene* scene) :
		ResourceGUID(guid),
		SceneContext(scene),
		Ptr(std::weak_ptr<GameObject>()),
		isNull(false)
	{ }

	bool GameObject::WeakRef::operator==(const GameObject::Sptr& other) {
		return Resolve() == other;
	}

	bool GameObject::WeakRef::operator!=(const GameObject::Sptr& other) {
		return Resolve() != other;
	}

	GameObject::Sptr GameObject::WeakRef::operator->() {
		return Resolve();
	}

	const GameObject::Sptr GameObject::WeakRef::operator->() const {
		return Resolve();
	}

	GameObject::Sptr GameObject::WeakRef::Resolve() const {
		// If we already determined the value is null, return null now
		if (isNull) { return nullptr; }

		// If the Ptr is uninitialized, try and look up the object in the scene
		if (GetIsEmpty()) {
			// We need a reference to the scene in order to search gameobjects :pensive:
			if (SceneContext != nullptr) {
				GameObject::Sptr result = SceneContext->FindObjectByGUID(ResourceGUID);
				Ptr = result;
				isNull = result == nullptr;
				return result;
			}
			// If there's no scene, return null
			else {
				isNull = true;
				return nullptr;
			}
		}
		// We've looked up the weak ptr, try and lock it
		else {
			return Ptr.lock();
		}
	}

	bool GameObject::WeakRef::GetIsEmpty() const {
		return !Ptr.owner_before(std::weak_ptr<GameObject>()) && !std::weak_ptr<GameObject>().owner_before(Ptr);
	}

	bool GameObject::WeakRef::IsAlive() const {
		return !GetIsEmpty() && !Ptr.expired();
	}

	void GameObject::WeakRef::Reset() {
		ResourceGUID = Guid();
		Ptr.reset();
		SceneContext = nullptr;
		isNull = true;
	}

	GameObject::WeakRef::operator GameObject::Sptr() const {
		return Resolve();
	}

	GameObject::WeakRef::operator Guid() const {
		return ResourceGUID;
	}

}
