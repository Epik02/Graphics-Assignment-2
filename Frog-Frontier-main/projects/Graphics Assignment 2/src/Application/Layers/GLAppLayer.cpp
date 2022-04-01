#include "Application/Layers/GLAppLayer.h"
#include "GLFW/glfw3.h"
#include "Logging.h"
#include "Application/Application.h"

GLAppLayer::GLAppLayer() :
	ApplicationLayer() {
	Name = "OpenGL Layer";
	Overrides = AppLayerFunctions::OnAppLoad | AppLayerFunctions::OnAppUnload;
}

GLAppLayer::~GLAppLayer() = default;

void GLAppLayer::OnAppLoad(const nlohmann::json& config) {
	// Initialize GLFW
	LOG_ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");

	Application& app = Application::Get();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a new GLFW window and make it current
	app._window = glfwCreateWindow(app._windowSize.x, app._windowSize.y, app._windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(app._window);

	// Set our window resized callback
	glfwSetWindowSizeCallback(app._window, GlWindowResizedCallback);

	LOG_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) != 0, "Failed to initialize glad");

	glEnable(GL_PROGRAM_POINT_SIZE);
}

void GLAppLayer::OnAppUnload()
{
	Application& app = Application::Get();
	glfwDestroyWindow(app._window);
	app._window = nullptr;
	app._windowSize = glm::ivec2(0, 0);
	app._windowTitle = "";

	glfwTerminate();
}

void GLAppLayer::GlDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
		case GL_DEBUG_SOURCE_API:             sourceTxt = "DEBUG"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceTxt = "WINDOW"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceTxt = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     sourceTxt = "APP"; break;
		case GL_DEBUG_SOURCE_OTHER: default:  sourceTxt = "OTHER"; break;
	}

	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
			#ifdef LOG_GL_NOTIFICATIONS
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
			#endif
		default: break;
	}
}

void GLAppLayer::GlWindowResizedCallback(GLFWwindow* window, int width, int height) {
	// Get the application singleton
	Application& app = Application::Get();

	// If the window was the applications, and the dimensions are non-zero, let the application handle window resize
	if (window == app._window && (width * height) > 0) {
		app.ResizeWindow(glm::ivec2(width, height));
	}
}
