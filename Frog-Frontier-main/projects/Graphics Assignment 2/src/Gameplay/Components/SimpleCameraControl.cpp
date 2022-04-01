#include "Gameplay/Components/SimpleCameraControl.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

SimpleCameraControl::SimpleCameraControl() :
	IComponent(),
	_mouseSensitivity({ 0.5f, 0.3f }),
	_moveSpeeds(glm::vec3(1.0f)),
	_shiftMultipler(2.0f),
	_currentRot(glm::vec2(0.0f)),
	_isMousePressed(false)
{ }

SimpleCameraControl::~SimpleCameraControl() = default;

void SimpleCameraControl::Update(float deltaTime)
{
	if (Application::Get().IsFocused) {
		if (InputEngine::GetMouseState(GLFW_MOUSE_BUTTON_LEFT) == ButtonState::Pressed) {
			_prevMousePos = InputEngine::GetMousePos();   
		}

		if (InputEngine::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			glm::dvec2 currentMousePos = InputEngine::GetMousePos();
			glm::dvec2 delta = currentMousePos - _prevMousePos;

			_currentRot.x += static_cast<float>(delta.x) * _mouseSensitivity.x;
			_currentRot.y += static_cast<float>(delta.y) * _mouseSensitivity.y;
			glm::quat rotX = glm::angleAxis(glm::radians(_currentRot.x), glm::vec3(0, 0, 1));
			glm::quat rotY = glm::angleAxis(glm::radians(_currentRot.y), glm::vec3(1, 0, 0));
			glm::quat currentRot = rotX * rotY;
			GetGameObject()->SetRotation(currentRot);

			_prevMousePos = currentMousePos;

			glm::vec3 input = glm::vec3(0.0f);
			if (InputEngine::IsKeyDown(GLFW_KEY_W)) {
				input.z -= _moveSpeeds.x;
			}
			if (InputEngine::IsKeyDown(GLFW_KEY_S)) {
				input.z += _moveSpeeds.x;
			}
			if (InputEngine::IsKeyDown(GLFW_KEY_A)) {
				input.x -= _moveSpeeds.y;
			}
			if (InputEngine::IsKeyDown(GLFW_KEY_D)) {
				input.x += _moveSpeeds.y;
			}
			if (InputEngine::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
				input.y -= _moveSpeeds.z;
			}
			if (InputEngine::IsKeyDown(GLFW_KEY_SPACE)) {
				input.y += _moveSpeeds.z;
			}

			if (InputEngine::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
				input *= _shiftMultipler;
			}

			input *= deltaTime;

			glm::vec3 worldMovement = currentRot * glm::vec4(input, 1.0f);
			GetGameObject()->SetPostion(GetGameObject()->GetPosition() + worldMovement);
		}
	}
	_prevMousePos = InputEngine::GetMousePos();
}

void SimpleCameraControl::RenderImGui()
{
	LABEL_LEFT(ImGui::DragFloat2, "Mouse Sensitivity", &_mouseSensitivity.x, 0.01f);
	LABEL_LEFT(ImGui::DragFloat3, "Move Speed       ", &_moveSpeeds.x, 0.01f, 0.01f);
	LABEL_LEFT(ImGui::DragFloat , "Shift Multiplier ", &_shiftMultipler, 0.01f, 1.0f);
}

nlohmann::json SimpleCameraControl::ToJson() const {
	return {
		{ "mouse_sensitivity", _mouseSensitivity },
		{ "move_speed", _moveSpeeds },
		{ "shift_mult", _shiftMultipler }
	};
}

SimpleCameraControl::Sptr SimpleCameraControl::FromJson(const nlohmann::json& blob) {
	SimpleCameraControl::Sptr result = std::make_shared<SimpleCameraControl>();
	result->_mouseSensitivity = JsonGet(blob, "mouse_sensitivity", result->_mouseSensitivity);
	result->_moveSpeeds       = JsonGet(blob, "move_speed", result->_moveSpeeds);
	result->_shiftMultipler   = JsonGet(blob, "shift_mult", 2.0f);
	return result;
}
