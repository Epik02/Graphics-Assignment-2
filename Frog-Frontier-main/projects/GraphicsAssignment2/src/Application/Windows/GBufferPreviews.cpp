#include "GBufferPreviews.h"
#include "Application/Application.h"
#include "../Layers/RenderLayer.h"
#include "Utils/ImGuiHelper.h"

GBufferPreviews::GBufferPreviews()
	: IEditorWindow()
{
	Name = "G-Buffer Previews";
	SplitDirection = ImGuiDir_::ImGuiDir_None;
	Requirements = EditorWindowRequirements::Window;
	Open = false;
}

GBufferPreviews::~GBufferPreviews() = default;

void GBufferPreviews::Render()
{
	using namespace Gameplay;
	Application& app = Application::Get();

	RenderLayer::Sptr& renderLayer = app.GetLayer<RenderLayer>();
	const Framebuffer::Sptr& framebuffer = renderLayer->GetGBuffer();
	const Framebuffer::Sptr& lightBuffer = renderLayer->GetLightingBuffer();

	Texture2D::Sptr& depth = framebuffer->GetTextureAttachment(RenderTargetAttachment::Depth);
	Texture2D::Sptr& color = framebuffer->GetTextureAttachment(RenderTargetAttachment::Color0);
	Texture2D::Sptr& normals = framebuffer->GetTextureAttachment(RenderTargetAttachment::Color1);
	Texture2D::Sptr& emissive = framebuffer->GetTextureAttachment(RenderTargetAttachment::Color2);
	Texture2D::Sptr& viewspace = framebuffer->GetTextureAttachment(RenderTargetAttachment::Color3);

	Texture2D::Sptr& diffuse = lightBuffer->GetTextureAttachment(RenderTargetAttachment::Color0);
	Texture2D::Sptr& specular = lightBuffer->GetTextureAttachment(RenderTargetAttachment::Color1);

	int width = (ImGui::GetContentRegionAvailWidth() / 2);
	float aspect = app.GetWindowSize().x / (float)app.GetWindowSize().y;
	int height = width / aspect;
	ImVec2 size = ImVec2(width, height);

	ImGui::Columns(2);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::BeginChildFrame(ImGui::GetID(depth.get()), ImVec2(size.x, size.y + ImGui::GetTextLineHeight() + 10));
	ImGuiHelper::DrawLinearDepthTexture(depth, glm::vec2(size.x, size.y), 0.01f, 1000.0f);
	ImGui::Text("depth");
	ImGui::EndChildFrame();
	ImGui::PopStyleVar();
	ImGui::NextColumn();

	_RenderTexture2D(color, size, "color");
	ImGui::NextColumn();

	_RenderTexture2D(normals, size, "normals");
	ImGui::NextColumn();

	_RenderTexture2D(emissive, size, "emissive"); 
	ImGui::NextColumn();  

	_RenderTexture2D(viewspace, size, "position (viewspace)");
	ImGui::NextColumn();

	_RenderTexture2D(diffuse, size, "Diffuse Lighting");
	ImGui::NextColumn();

	_RenderTexture2D(specular, size, "Specular Lighting");
	ImGui::NextColumn(); 

	ImGui::Columns(1);
}

void GBufferPreviews::_RenderTexture2D(const Texture2D::Sptr & value, const ImVec2& size, const char* name) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::BeginChildFrame(ImGui::GetID(value.get()), ImVec2(size.x, size.y + ImGui::GetTextLineHeight() + 10));
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	drawList->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
		glDisable(GL_BLEND);
	}, nullptr);
	ImGui::Image((ImTextureID)value->GetHandle(), size, ImVec2(0, 1), ImVec2(1, 0));
	drawList->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
		glEnable(GL_BLEND);
	}, nullptr);

	ImGui::Text(name);
	ImGui::EndChildFrame();
	ImGui::PopStyleVar();
}
