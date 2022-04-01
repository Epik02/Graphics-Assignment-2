#include "Gameplay/Components/GUI/RectTransform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/matrix_transform_2d.hpp>

#include "Utils/GlmDefines.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Graphics/GuiBatcher.h"

RectTransform::RectTransform() :
	_position({0.0f, 0.0f}),
	_halfSize({0.5f, 0.5f}),
	_rotation(0.0f),
	_transform(glm::mat3(1.0f)),
	_transformDirty(true)
{ }

RectTransform::~RectTransform() = default;

const glm::vec2& RectTransform::GetPosition() const {
	return _position;
}
void RectTransform::SetPosition(const glm::vec2& pos) {
	_position = pos;
	_transformDirty = true;
}

glm::vec2 RectTransform::GetMin() const {
	return _position - _halfSize;
}
void RectTransform::SetMin(const glm::vec2& value)
{
	// min = position - halfSize
	glm::vec2 newSize = glm::max(value, GetMax()) - glm::min(value, GetMax());
	_halfSize = newSize / 2.0f;
	_position = value + _halfSize;
	_transformDirty = true;
}

glm::vec2 RectTransform::GetMax() const {
	return _position + _halfSize;
}
void RectTransform::SetMax(const glm::vec2& value) {
	glm::vec2 newSize = glm::max(value, GetMin()) - glm::min(value, GetMin());
	_halfSize = newSize / 2.0f;
	_position = value - _halfSize;
	_transformDirty = true;
}

glm::vec2 RectTransform::GetSize() const {
	return _halfSize * 2.0f;
}
void RectTransform::SetSize(const glm::vec2& value) {
	_halfSize = value * 2.0f;
}

void RectTransform::SetRotationDeg(float value) {
	_rotation = glm::radians(value);
}

float RectTransform::GetRotationDeg() const {
	return glm::degrees(_rotation);
}

const glm::mat3& RectTransform::GetLocalTransform() const {
	__RecalcTransforms();
	return _transform;
}

void RectTransform::RenderImGui()
{
	_transformDirty |= LABEL_LEFT(ImGui::DragFloat2, "Position", &_position.x, 0.01f);
	float degrees = glm::degrees(_rotation);
	if (LABEL_LEFT(ImGui::DragFloat, "Rotation", &degrees, 0.1f)) {
		_transformDirty = true;
		_rotation = glm::radians(degrees);
	}
	glm::vec2 temp = GetSize();
	if (LABEL_LEFT(ImGui::DragFloat2, "Size    ", &temp.x, 0.1f)) {
		SetSize(temp);
	}
	temp = GetMin();
	if (LABEL_LEFT(ImGui::DragFloat2, "Min     ", &temp.x, 0.1f)) {
		SetMin(temp);
	}
	temp = GetMax();
	if (LABEL_LEFT(ImGui::DragFloat2, "Max     ", &temp.x, 0.1f)) {
		SetMax(temp);
	}
}

nlohmann::json RectTransform::ToJson() const {
	return {
		{ "position",   _position },
		{ "half_scale", _halfSize },
		{ "rotation",   _rotation }
	};
}

void RectTransform::StartGUI() {
	//GuiBatcher::PushScissorRect(_position - _halfSize, _position + _halfSize);
	GuiBatcher::PushModelTransform(GetLocalTransform());
}

void RectTransform::FinishGUI()
{
	GuiBatcher::PopModelTransform();
	//GuiBatcher::PopScissorRect();
}

RectTransform::Sptr RectTransform::FromJson(const nlohmann::json& blob)
{
	RectTransform::Sptr result = std::make_shared<RectTransform>();
	result->_position = JsonGet(blob, "position", result->_position);
	result->_halfSize = JsonGet(blob, "half_scale", result->_halfSize);
	result->_rotation = JsonGet(blob, "rotation", 0.0f);
	return result;
}

void RectTransform::__RecalcTransforms() const {
	if (_transformDirty) {
		_transform = glm::translate(MAT3_IDENTITY, _position) * glm::rotate(MAT3_IDENTITY, _rotation) * glm::translate(MAT3_IDENTITY, -_halfSize);
		_transformDirty = false;
	}
}
