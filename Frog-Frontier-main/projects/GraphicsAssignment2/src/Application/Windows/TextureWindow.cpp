#include "TextureWindow.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/Windows/FileDialogs.h"

TextureWindow::TextureWindow() :
	IEditorWindow()
{
	Name = "Textures";
	ParentName = "Materials";
	SplitDirection = ImGuiDir_::ImGuiDir_Right;
	SplitDepth = 0.5f;
}

TextureWindow::~TextureWindow() = default;

void TextureWindow::Render()
{
	int cols = glm::max((int)ImGui::GetContentRegionAvailWidth() / 64, 2);
	int size = (ImGui::GetContentRegionAvailWidth() / cols);
	ImGui::Columns(cols);
	ResourceManager::Each<Texture2D>([&](const Texture2D::Sptr& tex) {
		_RenderTexture2D(tex, size);
		ImGui::NextColumn(); 
	});
	if (ImGui::Button("Add Image", ImVec2(size, size))) {
		std::optional<std::string> file = FileDialogs::OpenFile("PNG\0*.png\0jpg\0*.jpg\0jpeg\0*.jpeg\0bmp\0*.bmp\0\0");

		if (file.has_value()) {
			ResourceManager::CreateAsset<Texture2D>(file.value());
		}
	}
	ImGui::Columns(1);
}

void TextureWindow::_RenderTexture3D(const Texture1D::Sptr& value, int width)
{

}

void TextureWindow::_RenderTexture2D(const Texture2D::Sptr& value, int width) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::BeginChildFrame(ImGui::GetID(value.get()), ImVec2(width, width + ImGui::GetTextLineHeight() + 10));
	ImGui::Image((ImTextureID)value->GetHandle(), ImVec2(width, width), ImVec2(0, 1), ImVec2(1, 0));
	ImGuiHelper::ResourceDragSource(value.get(), value->GetDebugName());
	ImGui::Text(value->GetDebugName().c_str());
	ImGui::EndChildFrame();
	ImGui::PopStyleVar();
}

void TextureWindow::_RenderTexture1D(const Texture3D::Sptr& value, int width)
{

}
