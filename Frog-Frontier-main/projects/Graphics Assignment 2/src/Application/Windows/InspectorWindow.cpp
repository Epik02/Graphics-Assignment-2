#include "InspectorWindow.h"
#include "../Application.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/GlmDefines.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/Scene.h"
#include "imgui_internal.h"

InspectorWindow::InspectorWindow() :
	IEditorWindow() 
{

	Name           = "Inspector";
	ParentName     = "Hierarchy";
	SplitDirection = ImGuiDir_::ImGuiDir_Down;
	SplitDepth     = 0.5f;
}
InspectorWindow::~InspectorWindow() = default;

void InspectorWindow::Render()
{
	Application& app = Application::Get();
	Gameplay::Scene::Sptr scene = app.CurrentScene();

	Gameplay::GameObject::Sptr selection = app.EditorState.SelectedObject.lock();
	if (selection != nullptr) {
		ImGui::PushID(selection.get());

		// Draw a textbox for the object name
		static char nameBuff[256];
		memcpy(nameBuff, selection->Name.c_str(), selection->Name.size());
		nameBuff[selection->Name.size()] = '\0';
		if (ImGui::InputText("##name", nameBuff, 256)) {
			selection->Name = nameBuff;
		}

		ImGui::Separator();

		// Render position label
		selection->_isLocalTransformDirty |= LABEL_LEFT(ImGui::DragFloat3, "Position", &selection->_position.x, 0.01f);

		// Get the ImGui storage state so we can avoid gimbal locking issues by storing euler angles in the editor
		glm::vec3 euler = selection->GetRotationEuler();
		ImGuiStorage* guiStore = ImGui::GetStateStorage();

		// Extract the angles from the storage, note that we're only using the address of the position for unique IDs
		euler.x = guiStore->GetFloat(ImGui::GetID(&selection->_position.x), euler.x);
		euler.y = guiStore->GetFloat(ImGui::GetID(&selection->_position.y), euler.y);
		euler.z = guiStore->GetFloat(ImGui::GetID(&selection->_position.z), euler.z);

		//Draw the slider for angles
		if (LABEL_LEFT(ImGui::DragFloat3, "Rotation", &euler.x, 1.0f)) {
			// Wrap to the -180.0f to 180.0f range for safety
			euler = Wrap(euler, -180.0f, 180.0f);

			// Update the editor state with our new values
			guiStore->SetFloat(ImGui::GetID(&selection->_position.x), euler.x);
			guiStore->SetFloat(ImGui::GetID(&selection->_position.y), euler.y);
			guiStore->SetFloat(ImGui::GetID(&selection->_position.z), euler.z);

			//Send new rotation to the gameobject
			selection->SetRotation(euler);
		}

		// Draw the scale
		selection->_isLocalTransformDirty |= LABEL_LEFT(ImGui::DragFloat3, "Scale   ", &selection->_scale.x, 0.01f, 0.0f);

		// For if we're not in play mode
		selection->_RecalcLocalTransform(); 
		selection->_RecalcWorldTransform();

		ImGui::Separator();

		// Render each component under it's own header
		for (int ix = 0; ix < selection->_components.size(); ix++) {
			std::shared_ptr<Gameplay::IComponent> component = selection->_components[ix];

			if (_RenderComponent(component)) {
				selection->_components.erase(selection->_components.begin() + ix);
				ix--;
			}
		}
		ImGui::Separator();

		// Render a combo box for selecting a component to add
		static std::string preview = "";
		static std::optional<std::type_index> selectedType;
		if (ImGui::BeginCombo("##AddComponents", preview.c_str())) {
			scene->Components().EachType([&](const std::string& typeName, const std::type_index type) {
				// Hide component types already added
				if (!selection->Has(type)) {
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
		if (ImGui::Button("Add Component") && selectedType.has_value() && !selection->Has(selectedType.value())) {
			selection->Add(selectedType.value());
			selectedType.reset();
			preview = "";
		}

		ImGui::PopID();
	}
}

bool InspectorWindow::_RenderComponent(Gameplay::IComponent::Sptr component)
{
	ImGui::PushID(component.get());

	ImGuiID deletePopup = ImGui::GetID("Delete Component##INSPECTOR_DELETE");

	ImGuiID id = ImGui::GetID(component->ComponentTypeName().c_str());
	bool isOpen = ImGui::CollapsingHeader(component->ComponentTypeName().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_ClipLabelForTrailingButton);
	ImGuiHelper::HeaderCheckbox(id, &component->IsEnabled);
	
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Copy Values")) {

		}
		if (ImGui::MenuItem("Paste Values")) {

		}

		ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
		if (ImGui::MenuItem("Delete")) {
			ImGui::OpenPopupEx(deletePopup);
		}
		ImGui::PopStyleColor(3);

		ImGui::EndPopup();
	}


	if (ImGui::BeginPopupModal("Delete Component##INSPECTOR_DELETE")) {
		ImGui::Text("Are you sure you want to delete this component? This cannot be undone");
		if (ImGuiHelper::WarningButton("Yes")) {
			// Restore imgui state so we can early bail
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			ImGui::PopID();
			return true;
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();

	}

	if (isOpen) {

		ImGui::Indent();
		component->RenderImGui();
		ImGui::Unindent();
	}

	ImGui::Separator();
	ImGui::PopID();

	return false;
}
