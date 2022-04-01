#pragma once
#include <string>
#include <GLM/glm.hpp>
#include <json.hpp>
#include "Utils/Macros.h"
#include "Application/ApplicationLayer.h"
#include "Gameplay/Scene.h"

struct GLFWwindow;

/**
 * The application will be the main container for all of our shared game engine features,
 * such as windows, input, rendering, etc...
 */
class Application final {
public:
	NO_MOVE(Application)
	NO_COPY(Application)

	bool IsFocused = true;

	~Application();

	// Information about the application for when we are in editor mode
	struct {
		bool IsEditor = true;
		Gameplay::GameObject::Wptr SelectedObject;
	} EditorState;

	static Application& Get();
	/**
	 * Called by the entry point to begin the application, creating the singleton 
	 * intance and performing any library initialization
	 */
	static void Start(int argCount, char** arguments);

	/**
	 * Gets the GLFW window for the application
	 */
	GLFWwindow* GetWindow();

	/**
	 * Gets the width and height of the application window, in pixels
	 */
	const glm::ivec2& GetWindowSize() const;
	/**
	 * Resizes the application window to the given size in pixels
	 * 
	 * @param newSize The new size for the window, in pixels. Should not contain zeroes or negative values
	 */
	void ResizeWindow(const glm::ivec2& newSize);

	/**
	 * Gets the viewport that the game will appear in on the screen. For non-editor versions, this should be the entire screen
	 * @returns The bounds for the viewport, in screenspace, formatted as { x, y, width, height }
	 */
	const glm::uvec4& GetPrimaryViewport() const;
	/**
	 * Sets the viewport that the game will appear in on the screen. For non-editor versions, this should be the entire screen
	 * @param value The bounds for the viewport, in screenspace, formatted as { x, y, width, height }
	 */
	void SetPrimaryViewport(const glm::uvec4& value);

	/**
	 * Quits the application at the end of the current frame
	 */
	void Quit();

	/**
	 * Loads a new scene into the application using a path on disk
	 * 
	 * @param path The path to the scene file to load
	 * @returns True if the file was found and the scene loaded, false if otherwise
	 */
	bool LoadScene(const std::string& path);
	/**
	 * Loads a scene that has been pre-loaded into the application
	 * 
	 * @param scene The scene to switch to
	 */
	void LoadScene(const Gameplay::Scene::Sptr& scene);

	/**
	 * Gets the currently loaded scene that the application is working from
	 */
	Gameplay::Scene::Sptr CurrentScene() { return _currentScene == nullptr ? _targetScene : _currentScene; }

	/**
	 * Gets the layer of the given type from the application, or nullptr if it does not exist
	 * 
	 * @param <T> The type of layer to get, must extend the ApplicationLayer interface
	 * @returns A shared pointer to the layer instance, or nullptr if the layer cannot be found
	 */
	template <typename T, typename = typename std::enable_if<std::is_base_of<ApplicationLayer, T>::value>::type>
	std::shared_ptr<T> GetLayer() {
		// Iterate over all the pointers in the layers list
		for (const auto& ptr : _layers) {
			// If the pointer type matches T, we return that layer, making sure to cast it back to the requested type
			if (std::type_index(typeid(*ptr.get())) == std::type_index(typeid(T))) {
				return std::dynamic_pointer_cast<T>(ptr);
			}
		}
		return nullptr;
	}
	
	/**
	 * Saves the application settings to a file in %APPDATA% 
	 */
	void SaveSettings();

protected:
	// The GL driver layer is a special friend that can access our protected members (mainly window info)
	friend class GLAppLayer;

	Application();

	// The GLFW window that the application will render to and receive events from
	GLFWwindow* _window;
	// The title of the above window, will appear when not in fullscreen mode
	std::string _windowTitle;
	// The size of the game window, in pixels
	glm::ivec2  _windowSize;
	// True as long as the application is running
	bool        _isRunning;

	// Not an idea way of distinguising, since we need to build editor into our game, but good 'nuff for GDW
	bool        _isEditor;

	// The primary viewport that the game will render into, in client window bounds
	glm::uvec4  _primaryViewport;

	// Stores the current application settings
	nlohmann::json _appSettings;

	// The current scene that the application is working on
	Gameplay::Scene::Sptr _currentScene;
	// The scene to switch to at the start of the next frame
	Gameplay::Scene::Sptr _targetScene;

	// Stores all the layers of the application, in the order they should be invoked
	std::vector<ApplicationLayer::Sptr> _layers;

	void _Run();
	void _RegisterClasses();
	void _Load();
	void _Update();
	void _LateUpdate();
	void _PreRender();
	void _RenderScene();
	void _PostRender();
	void _Unload();
	void _HandleSceneChange();
	void _HandleWindowSizeChanged(const glm::ivec2& newSize);
	void _ConfigureSettings();
	nlohmann::json _GetDefaultAppSettings();

	static Application* _singleton;
	static std::string  _applicationName;
};