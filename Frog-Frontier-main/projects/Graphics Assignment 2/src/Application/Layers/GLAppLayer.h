#pragma once
#include "Application/ApplicationLayer.h"
#include <glad/glad.h>
#include <json.hpp>

struct GLFWwindow;

/**
 * The GL Application Layer will handle initializing and cleaning up OpenGL and GLFW, as well as handling
 * things like debug and window size callbacks
 */
class GLAppLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(GLAppLayer)

	GLAppLayer();
	virtual ~GLAppLayer();

	// Inherited from ApplicationLayer

	virtual void OnAppLoad(const nlohmann::json& config) override;
	virtual void OnAppUnload() override;

protected:
	static void GlDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	static void GlWindowResizedCallback(GLFWwindow* window, int width, int height);
};