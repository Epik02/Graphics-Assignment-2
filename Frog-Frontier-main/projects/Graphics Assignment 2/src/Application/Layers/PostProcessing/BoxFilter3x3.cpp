#include "BoxFilter3x3.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Graphics/Framebuffer.h"

#include <GLM/glm.hpp>

BoxFilter3x3::BoxFilter3x3() :
	PostProcessingLayer::Effect()
{
	Name = "Box Filter";
	_format = RenderTargetType::ColorRgb8;

	// Zero the memory, then set center pixel to 1.0
	memset(Filter, 0, sizeof(float) * 9);
	Filter[4] = 1.0f;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/box_filter_3.glsl" }
	});
}

BoxFilter3x3::~BoxFilter3x3() = default;

void BoxFilter3x3::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind(); 
	_shader->SetUniform("u_Filter", Filter, 9); 
	_shader->SetUniform("u_PixelSize", glm::vec2(1.0f) / (glm::vec2)gBuffer->GetSize()); 
}

void BoxFilter3x3::RenderImGui()
{
	ImGui::PushID(this);

	ImGui::Columns(3); 
	for (int iy = 0; iy < 3; iy++) { 
		for (int ix = 0; ix < 3; ix++) {
			ImGui::PushID(iy * 3 + ix);
			ImGui::PushItemWidth(-1);
			ImGui::InputFloat("", &Filter[iy * 3 + ix], 0.01f);
			ImGui::PopItemWidth();
			ImGui::PopID();
			ImGui::NextColumn();
		}
	}
	ImGui::Columns(1);

	if (ImGui::Button("Normalize")) {
		float sum = 0.0f;
		for (int ix = 0; ix < 9; ix++) {
			sum += Filter[ix];
		}
		float mult = sum == 0.0f ? 1 : 1.0f / sum;

		for (int ix = 0; ix < 9; ix++) {
			Filter[ix] *= mult;
		}
	}

	float* temp = ImGui::GetStateStorage()->GetFloatRef(ImGui::GetID("###temp-filler"), 0.0f);
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.75f);
	ImGui::InputFloat("", temp, 0.1f);
	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("Fill")) {
		for (int ix = 0; ix < 9; ix++) {
			Filter[ix] = *temp;
		}
	}

	ImGui::PopID();
}

BoxFilter3x3::Sptr BoxFilter3x3::FromJson(const nlohmann::json& data)
{
	BoxFilter3x3::Sptr result = std::make_shared<BoxFilter3x3>();
	result->Enabled = JsonGet(data, "enabled", true);
	std::vector<float> filter = JsonGet(data, "filter", std::vector<float>(9, 0.0f));
	for (int ix = 0; ix < 9; ix++) {
		result->Filter[ix] = filter[ix];
	}
	return result;
}

nlohmann::json BoxFilter3x3::ToJson() const
{
	std::vector<float> filter;
	for (int ix = 0; ix < 9; ix++) {
		filter.push_back(Filter[ix]);
	}
	return {
		{ "enabled", Enabled },
		{ "filter", filter }
	};
}
