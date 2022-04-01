#include "ShadowCamera.h"
#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ImGuiHelper.h"
#include "imgui_internal.h"
#include "GLM/gtc/matrix_transform.hpp"

ShadowCamera::ShadowCamera() :
	Flags(ShadowFlags::None),
	Bias(0.00001f),
	NormalBias(0.0001f),
	Intensity(1.0f),
	Range(100.0f),
	_depthBuffer(nullptr),
	_projectionMask(nullptr),
	_color(glm::vec4(1.0f)),
	_bufferResolution(glm::ivec2(512)), 
	_projectionMatrix(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f))
{ }

ShadowCamera::~ShadowCamera() = default;

void ShadowCamera::SetColor(const glm::vec4& value) {
	_color = value;
}

const glm::vec4& ShadowCamera::GetColor() const {
	return _color;
}

void ShadowCamera::SetBufferResolution(const glm::ivec2& value) {
	LOG_ASSERT(value.x * value.y > 0, "Buffer size must be > 0");
	_bufferResolution = value;
	if (_depthBuffer != nullptr) {
		_depthBuffer->Resize(value);
	}
}

const glm::ivec2& ShadowCamera::GetBufferResolution() const {
	return _bufferResolution;
}

void ShadowCamera::SetProjection(const glm::mat4& value) {
	_projectionMatrix = value;
}

const glm::mat4& ShadowCamera::GetProjection() const {
	return _projectionMatrix;
}

glm::mat4 ShadowCamera::GetViewProjection() const
{
	return _projectionMatrix * GetGameObject()->GetInverseTransform();
}

void ShadowCamera::SetProjectionMask(const Texture2D::Sptr& image) {
	_projectionMask = image;

	// Update our render flags
	Flags = (Flags & ~*ShadowFlags::ProjectionEnabled) | (image != nullptr ? ShadowFlags::ProjectionEnabled : ShadowFlags::None);
}

const Texture2D::Sptr& ShadowCamera::GetProjectionMask() const {
	return _projectionMask;
}

void ShadowCamera::OnLoad()
{
	LOG_ASSERT(_bufferResolution.x * _bufferResolution.y > 0, "Buffer size must be > 0");

	FramebufferDescriptor desc;
	desc.Width  = _bufferResolution.x;
	desc.Height = _bufferResolution.y;
	desc.RenderTargets[RenderTargetAttachment::Depth] = RenderTargetDescriptor(RenderTargetType::Depth32);

	_depthBuffer = std::make_shared<Framebuffer>(desc);
}

nlohmann::json ShadowCamera::ToJson() const
{
	return {
		{ "color", _color },
		{ "bias", Bias },
		{ "normal_bias", NormalBias },
		{ "range", Range },
		{ "intensity", Intensity },
		{ "resolution", _bufferResolution },
		{ "flags", *Flags },
		{ "mask", _projectionMask ? _projectionMask->GetGUID().str() : "null" },
		{ "projection", _projectionMatrix }
	};
}

ShadowCamera::Sptr ShadowCamera::FromJson(const nlohmann::json& data)
{
	ShadowCamera::Sptr result = std::make_shared<ShadowCamera>();

	result->Flags = (ShadowFlags)JsonGet<uint32_t>(data, "flags", *result->Flags);
	result->Bias = JsonGet(data, "bias", result->Bias);
	result->NormalBias = JsonGet(data, "normal_bias", result->NormalBias);
	result->Range = JsonGet(data, "range", result->Range);
	result->Intensity = JsonGet(data, "intensity", result->Intensity);
	result->_color = JsonGet(data, "color", result->_color);
	result->_bufferResolution = JsonGet(data, "resolution", result->_bufferResolution);
	result->_projectionMask = ResourceManager::Get<Texture2D>(Guid(JsonGet<std::string>(data, "mask", "null")));
	result->_projectionMatrix = JsonGet(data, "projection", result->_projectionMatrix);
	return result;
}

const Framebuffer::Sptr& ShadowCamera::GetDepthBuffer() const
{
	return _depthBuffer;
}

void ShadowCamera::RenderImGui()
{
	ImGui::PushID(this);

	ImGui::Separator();
	ImGui::Text("Shadow Settings");
	ImGui::Separator();

	if (ImGui::BeginCombo("Flags", (~Flags).c_str())) {

		ImGui::CheckboxFlags("PCF", (uint32_t*)&Flags, *ShadowFlags::PcfEnabled);
		ImGui::CheckboxFlags("Wide PCF", (uint32_t*)&Flags, *ShadowFlags::WidePcfEnabled);
		ImGui::CheckboxFlags("Attenuation", (uint32_t*)&Flags, *ShadowFlags::AttenuationEnabled);

		ImGui::EndCombo();
	}
	ImGui::DragFloat("Bias", &Bias, 0.000001f, 0.0f, 0.1f, "%.9f");
	ImGui::DragFloat("Normal Bias", &NormalBias, 0.000001f, 0.0f, 0.1f, "%.9f");
	if (ImGui::DragInt2("Resolution", &_bufferResolution.x, 1.0f, 1, 1024)) {
		SetBufferResolution(_bufferResolution);
	}

	// Projection Mask
	{
		ImGui::Text("Projector");
		ImGui::SameLine();
		if (ImGuiHelper::DrawTextureDrop(_projectionMask, ImVec2(ImGui::GetTextLineHeight() * 2, ImGui::GetTextLineHeight() * 2))) {
			Flags |= ShadowFlags::ProjectionEnabled;
		}

		if (_projectionMask != nullptr) {
			ImGui::SameLine();
			if (ImGui::SmallButton("x")) {
				SetProjectionMask(nullptr);
			}
		}
	}

	ImGui::Separator();
	ImGui::Text("Light Settings");
	ImGui::Separator();

	ImGui::ColorEdit4("Color", &_color.x);
	ImGui::DragFloat("Range", &Range, 0.01f, 0.0f, 1000.0f);
	ImGui::DragFloat("Intensity", &Intensity, 0.01f, 0.0f, 1000.0f);

	ImGui::Separator();
	ImGui::Text("Debugging");
	ImGui::Separator();

	// Depth display
	{
		bool checked = ImGui::GetStateStorage()->GetBool(ImGui::GetID("show_depth"), false);
		if (ImGui::Checkbox("Show Depth", &checked)) {
			ImGui::GetStateStorage()->SetBool(ImGui::GetID("show_depth"), checked);
		}
		if (_depthBuffer != nullptr && checked) {
			Texture2D::Sptr depth = _depthBuffer->GetTextureAttachment(RenderTargetAttachment::Depth);

			int width = ImGui::GetContentRegionAvailWidth();

			ImGui::Columns(1);
			ImGuiHelper::DrawLinearDepthTexture(depth, glm::ivec2(width, width), 0.1f, 100.0f);
		}
	}

	ImGui::PopID();
}
