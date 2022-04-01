#include "Gameplay/InputEngine.h"
#include <locale>
#include <codecvt>
#include "Application/Application.h"

GLFWwindow* InputEngine::__window = nullptr;
glm::dvec2 InputEngine::__mousePos  = glm::dvec2(0.0);
glm::dvec2 InputEngine::__prevMousePos = glm::dvec2(0.0);;
glm::dvec2 InputEngine::__scrollDelta = glm::dvec2(0.0);
std::wstring InputEngine::__inputText = LR"()";

ButtonState InputEngine::__mouseState[GLFW_MOUSE_BUTTON_LAST + 1];
ButtonState InputEngine::__keyState[GLFW_KEY_LAST + 1];

void InputEngine::Init(GLFWwindow* window)
{
	__window = window;
	glfwSetCharCallback(__window, InputEngine::__CharCallback);
	glfwSetMouseButtonCallback(__window, InputEngine::__MouseButtonCallback);
	glfwSetKeyCallback(__window, InputEngine::__KeyCallback);
	glfwSetScrollCallback(__window, InputEngine::__MouseScrollCallback);
}

ButtonState InputEngine::GetKeyState(int keyCode) {
	return (keyCode <= GLFW_KEY_LAST) ? __keyState[keyCode] : ButtonState::Up;
}

ButtonState InputEngine::GetMouseState(int button)
{
	return (button <= GLFW_MOUSE_BUTTON_LAST) ? __mouseState[button] : ButtonState::Up;
}

bool InputEngine::IsKeyDown(int keyCode) {
	return (keyCode <= GLFW_KEY_LAST) ? *__keyState[keyCode] & 0b01 : false;
}

bool InputEngine::IsMouseButtonDown(int button) {
	return (button <= GLFW_MOUSE_BUTTON_LAST) ? *__mouseState[button] & 0b01 : false;
}

const glm::dvec2& InputEngine::GetMousePos() {
	Application& app = Application::Get();
	glm::vec4 viewport = app.GetPrimaryViewport();
	return (__mousePos - glm::dvec2(viewport.x, viewport.y));
}

glm::dvec2 InputEngine::GetMouseDelta() {
	return __mousePos - __prevMousePos;
}

void InputEngine::SetCursorMode(CursorMode mode) {
	glfwSetInputMode(__window, GLFW_CURSOR, *mode);
}

std::wstring InputEngine::GetInputText() {
	return __inputText;
}

std::string InputEngine::GetInputTextAscii() {
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> StringConvert;
	return StringConvert.to_bytes(__inputText);
}

void InputEngine::EndFrame() {
	__prevMousePos = __mousePos;
	glfwGetCursorPos(__window, &__mousePos.x, &__mousePos.y);

	__scrollDelta.x = __scrollDelta.y = 0.0;
	__inputText.clear();

	// Since we used a bit field for our enum values, we can do a quick and
	// to convert from pressed or released to down/up
	for (int ix = 0; ix < GLFW_KEY_LAST + 1; ix++) {
		__keyState[ix] = (ButtonState)(*__keyState[ix] & 0b01);
	}

	for (int ix = 0; ix < GLFW_MOUSE_BUTTON_LAST + 1; ix++) {
		__mouseState[ix] = (ButtonState)(*__mouseState[ix] & 0b01);
	}
}


void InputEngine::__KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_UNKNOWN)
		return;

	switch (action) {
		case GLFW_PRESS:
			__keyState[key] = ButtonState::Pressed;
			break;
		case GLFW_RELEASE:
			__keyState[key] = ButtonState::Released;
			break;
		default:
			break;
	}
}

void InputEngine::__CharCallback(GLFWwindow* window, uint32_t keycode) {
	__inputText.push_back(keycode);
}

void InputEngine::__MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button > GLFW_MOUSE_BUTTON_LAST)
		return;

	if (action == GLFW_PRESS) {
		__mouseState[button] = ButtonState::Pressed;
	} else if (action == GLFW_RELEASE) {
		__mouseState[button] = ButtonState::Released;
	}
}

void InputEngine::__MouseScrollCallback(GLFWwindow* window, double x, double y) {
	__scrollDelta.x += x;
	__scrollDelta.y += y;
}
