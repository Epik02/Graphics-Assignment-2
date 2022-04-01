#pragma once

#include <GLM/glm.hpp>
#include <string>
#include <EnumToString.h>
#include "GLFW/glfw3.h"

ENUM_FLAGS(ButtonState, int,
	 Up       = 0b00,
	 Down     = 0b01,
	 Pressed  = 0b11,
	 Released = 0b10
);

ENUM(CursorMode, int,
	 Normal = GLFW_CURSOR_NORMAL,
	 Disabled = GLFW_CURSOR_DISABLED,
	 Hidden   = GLFW_CURSOR_HIDDEN
);

class InputEngine {
public:
	static void Init(GLFWwindow* window);

	static ButtonState GetKeyState(int keyCode);
	static ButtonState GetMouseState(int button);

	static bool IsKeyDown(int keyCode);
	static bool IsMouseButtonDown(int button);

	static const glm::dvec2& GetMousePos();
	static glm::dvec2 GetMouseDelta();

	static void SetCursorMode(CursorMode mode);

	static std::wstring GetInputText();
	static std::string  GetInputTextAscii();

	static void EndFrame();

private:
	static GLFWwindow*  __window;
	static ButtonState  __keyState[GLFW_KEY_LAST + 1];
	static ButtonState  __mouseState[GLFW_MOUSE_BUTTON_LAST + 1];
	static glm::dvec2   __mousePos;
	static glm::dvec2   __prevMousePos;
	static glm::dvec2   __scrollDelta;
	static std::wstring __inputText;

	static void __KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void __CharCallback(GLFWwindow* window, uint32_t keycode);
	static void __MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void __MouseScrollCallback(GLFWwindow* window, double x, double y);
};