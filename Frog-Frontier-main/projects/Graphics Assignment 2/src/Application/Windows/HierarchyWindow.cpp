#include "HierarchyWindow.h"
#include "../Application.h"
#include "Utils/ImGuiHelper.h"
#include "imgui_internal.h"
#include "Graphics/DebugDraw.h"

HierarchyWindow::HierarchyWindow() :
	IEditorWindow()
{
	Name           = "Hierarchy";
	SplitDirection = ImGuiDir_::ImGuiDir_Right;
	SplitDepth     = 0.2f;
}

HierarchyWindow::~HierarchyWindow() = default;

void HierarchyWindow::Render()
{
	Application& app = Application::Get();

	// Render a popup on right-click
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Add New Object")) {
			app.CurrentScene()->CreateGameObject("GameObject");
		}

		ImGui::EndPopup();
	}
	
	for (const auto& object : app.CurrentScene()->_objects) {
		_RenderObjectNode(object);
	}
}

void HierarchyWindow::_RenderObjectNode(Gameplay::GameObject::Sptr object, int depth) {
	using namespace Gameplay;

	if (object->HideInHierarchy) {
		return;
	}

	// If the parent exists and we're at depth 0, abort
	if (object->GetParent() != nullptr && depth == 0) {
		return;
	}

	Application& app = Application::Get();
	Scene::Sptr& scene = app.CurrentScene();

	ImGui::PushID(object->GetGUID().str().c_str());

	// Figure out how the object node should be displayed
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	GameObject::Sptr selectedObject = app.EditorState.SelectedObject.lock();

	if (selectedObject != nullptr && selectedObject == object) {
		flags |= ImGuiTreeNodeFlags_Selected;

		glm::mat3 rot = glm::mat3(1.0f);

		// Todo: some kinda toggle to switch this, and an actual gizmos system lol
		rot = glm::mat3(object->GetTransform());

		glm::vec3 forward = glm::normalize(rot * glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 right   = glm::normalize(rot * glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 up      = glm::normalize(rot * glm::vec3(0.0f, 0.0f, 1.0f));

		DebugDrawer::Get().DrawLine(
			object->GetWorldPosition(), 
			object->GetWorldPosition() + (glm::vec3)forward,
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
		);
		DebugDrawer::Get().DrawLine(
			object->GetWorldPosition(),
			object->GetWorldPosition() + (glm::vec3)right,
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
		);
		DebugDrawer::Get().DrawLine(
			object->GetWorldPosition(),
			object->GetWorldPosition() + (glm::vec3)up,
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
		);
	}
	if (object->GetChildren().size() == 0) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	// Determine the text of the node
	static char buffer[256];
	sprintf_s(buffer, 256, "%s###GO_HEADER", object->Name.c_str());
	bool isOpen = ImGui::TreeNodeEx(buffer, flags);
	if (ImGui::IsItemClicked()) {
		// TODO: Properly handle multi-selection
		//ImGuiIO& io = ImGui::GetIO();
		//if (!io.KeyShift) {
		//	_selectedObjects.clear();
		//}

		app.EditorState.SelectedObject = object;
	}

	// We need to get the ID of the modal out here, since the menu item will push new 
	// IDs to the stack and cause issues
	ImGuiID deletePopup = ImGui::GetID("###HIERARCHY_DELETE");

	// Render a popup on right-click
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Add Child")) {
			object->AddChild(object->GetScene()->CreateGameObject("GameObject"));
		}
		if (ImGui::MenuItem("Delete")) {
			ImGui::OpenPopupEx(deletePopup);
		}

		ImGui::Separator();

		char buffer[64];
		scene->Components().EachType([&](const std::string& typeName, const std::type_index type) {
			// Hide component types already added
			sprintf_s(buffer, "Add %s", typeName.c_str());
			if (ImGui::MenuItem(buffer, nullptr, nullptr, !object->Has(type))) {
				object->Add(type);
			}
		});

		ImGui::EndPopup();
	}

	// Draw our delete modal
	if (ImGui::BeginPopupModal("Delete Gameobject###HIERARCHY_DELETE")) {
		ImGui::Text("Are you sure you want to delete this game object?");
		if (ImGuiHelper::WarningButton("Yes")) {
			// Remove ourselves from the scene
			object->GetScene()->RemoveGameObject(object);

			// If the parent is not null, let it know we just killed it's child
			if (object->GetParent() != nullptr) {
				object->GetParent()->_PurgeDeletedChildren();
			}

			// Restore imgui state so we can early bail
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			ImGui::PopID();
			if (isOpen) {
				ImGui::TreePop();
			}
			return;
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();

	}

	// If the node is expanded, render the child nodes
	if (isOpen) {
		for (const auto& child : object->GetChildren()) {
			_RenderObjectNode(child, depth + 1);
		}

		// We also need the tree pop so that we finish the tree node
		ImGui::TreePop();
	}


	ImGui::PopID();
}
