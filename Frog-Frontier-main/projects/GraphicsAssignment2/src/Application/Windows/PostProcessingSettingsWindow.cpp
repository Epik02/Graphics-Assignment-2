#include "PostProcessingSettingsWindow.h"
#include "../Application.h"
#include "Utils/ImGuiHelper.h"
#include "imgui_internal.h"
#include <set>

PostProcessingSettingsWindow::PostProcessingSettingsWindow()
	: IEditorWindow()
{
	Name = "Post Processing Effects";
	SplitDirection = ImGuiDir_::ImGuiDir_None;
	Requirements = EditorWindowRequirements::Window;
	Open = false;
}

PostProcessingSettingsWindow::~PostProcessingSettingsWindow() = default;

void PostProcessingSettingsWindow::Render()
{
	using namespace Gameplay;
	Application& app = Application::Get();

	PostProcessingLayer::Sptr layer = app.GetLayer<PostProcessingLayer>();

	std::set<PostProcessingLayer::Effect::Sptr> unique (layer->GetEffects().begin(), layer->GetEffects().end());

	for (const auto& effect : unique) {
		_RenderEffect(effect);
	}
}

void PostProcessingSettingsWindow::_RenderEffect(const PostProcessingLayer::Effect::Sptr& value)
{
	ImGui::PushID(value.get());

	ImGuiID id = ImGui::GetID(value->Name.c_str());
	bool isOpen = ImGui::CollapsingHeader(value->Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_ClipLabelForTrailingButton);
	ImGuiHelper::HeaderCheckbox(id, &value->Enabled);

	if (isOpen) {
		ImGui::Indent();
		value->RenderImGui();
		ImGui::Unindent();
		ImGui::Separator();
	}

	ImGui::PopID();
}
