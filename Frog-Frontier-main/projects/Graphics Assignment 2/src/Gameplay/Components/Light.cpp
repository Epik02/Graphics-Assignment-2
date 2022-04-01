#include "Light.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

Light::Light() : 
	Gameplay::IComponent(),
	_color(glm::vec3(1.0f)),
	_direction(glm::vec3(0.0f, 0.0f, -1.0f)),
	_params(glm::vec3(0.0f)),
	_radius(1.0f),
	_intensity(10.0f),
	_type(LightType::Point)
{

}

Light::~Light() = default;

const glm::vec3& Light::GetColor() const {
	return _color;
}

void Light::SetColor(const glm::vec3& value){
	_color = value;
}

const glm::vec3& Light::GetDirection() const {
	return _direction;
}

void Light::SetDirection(const glm::vec3& value) {
	_direction = value;
}

const glm::vec3& Light::GetParams() const {
	return _params;
}

void Light::SetParams(const glm::vec3& value) {
	_params = value;
}

float Light::GetIntensity() const {
	return _intensity;
}

void Light::SetIntensity(float value) {
	_intensity = value;
}

float Light::GetRadius() const {
	return _radius;
}

void Light::SetRadius(float value) {
	_radius = value;
}

LightType Light::GetType() const {
	return _type;
}

void Light::SetType(LightType value) {
	_type = value;
}

/// <summary>
/// Loads a light from a JSON blob
/// </summary>
Light::Sptr Light::FromJson(const nlohmann::json& data) {
	Light::Sptr result = std::make_shared<Light>();
	result->_color = JsonGet(data, "color", result->_color);
	result->_radius = JsonGet(data, "range", result->_radius);
	result->_direction = JsonGet(data, "direction", result->_direction);
	result->_params = JsonGet(data, "params", result->_params);
	result->_intensity = JsonGet(data, "intensity", result->_intensity);
	result->_type = JsonParseEnum(LightType, data, "type", LightType::Point);
	return result;
}

/// <summary>
/// Converts this object into it's JSON representation for storage
/// </summary>
inline nlohmann::json Light::ToJson() const {
	return {
		{ "color", _color },
		{ "range", _radius },
		{ "direction", _direction },
		{ "params", _params },
		{ "type", ~_type },
		{ "intensity", _intensity }
	};
}

void Light::RenderImGui()
{
	LABEL_LEFT(ImGui::ColorPicker3, "    Color", &_color.x);
	LABEL_LEFT(ImGui::DragFloat,    "    Range", &_radius, 0.1f, 0.0f);
	LABEL_LEFT(ImGui::DragFloat,    "Intensity", &_intensity, 0.1f, 0.0f);
	LABEL_LEFT(ImGui::DragFloat3,   "Direction", &_direction.x, 0.01f, -1.0f, 1.0f);
	LABEL_LEFT(ImGui::DragFloat3,   "   Params", &_params.x, 0.01f);

	ENUM_COMBO("     Type", &_type, LightType);

}
