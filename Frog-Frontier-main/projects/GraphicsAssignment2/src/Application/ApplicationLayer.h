#pragma once
#include <string>
#include <EnumToString.h>
#include "Utils/Macros.h"
#include <json.hpp>
#include <GLM/glm.hpp>

#include "Graphics/Framebuffer.h"

/**
 * Enumeration flags that let the application know what functions a layer has overriden,
 * can be used for a bit of a performance boost (since calling empty virtual functions can
 * still have way more overhead than a flag check, and may be hard for the compiler to optimize)
 */
ENUM_FLAGS(AppLayerFunctions, uint32_t,
	None = 0,
	OnAppLoad      = 1 << 0,
	OnAppUnload    = 1 << 1,
	OnSceneLoad    = 1 << 2,
	OnSceneUnload  = 1 << 3,
	OnUpdate       = 1 << 4,
	OnLateUpdate   = 1 << 5,
	OnPreRender    = 1 << 6,
	OnRender       = 1 << 7,
    OnPostRender   = 1 << 8,
	OnWindowResize = 1 << 9,

	All = 0xFFFFFFFF
)

/**
 * An application layer is a container class for related systems within our game.
 * For instance, handling physics may be a layer, rendering may be another, 
 * scene updates another, etc...
 */
class ApplicationLayer {
public:
	MAKE_PTRS(ApplicationLayer);
	NO_MOVE(ApplicationLayer);
	NO_COPY(ApplicationLayer);

	/**
	 * When enabled, all commands in the layer will be invoked by the application
	 */
	bool Enabled = true;
	/**
	 * A human readable name for the layer, for debugging purposes
	 */
	std::string Name;
	/**
	 * Tells the application which functions should be invoked for this layer
	 */
	AppLayerFunctions Overrides = AppLayerFunctions::All;

	virtual ~ApplicationLayer() = default;

	/**
	 * Invoked when the application is first loaded
	 * 
	 * @param config The global application config
	 */
	virtual void OnAppLoad(const nlohmann::json& config) {};
	/**
	 * Invoked when the application has quit, and is unloading resources
	 */
	virtual void OnAppUnload() {};

	/**
	 * Invoked after a new scene has been loaded
	 */
	virtual void OnSceneLoad() {};
	/**
	 * Invoked before a scene begins it's unload process
	 */
	virtual void OnSceneUnload() {};

	/**
	 * Invoked when the application updates, at varying time steps (see Timing class)
	 */
	virtual void OnUpdate() {};
	/**
	 * Invoked after all layers in an application have been updated
	 */
	virtual void OnLateUpdate() {};

	virtual void OnPreRender() {};
	/**
	 * Invoked after LateUpdate, allows layers to perform rendering
	 */
	virtual void OnRender(const Framebuffer::Sptr& prevLayer) {};

	virtual void OnPostRender() {};

	/**
	 * Allows the layer to handle when the application window has been resized
	 * 
	 * @param oldSize The previous size of the application window, in pixels
	 * @param newSize The new size of the application window, in pixels
	 */
	virtual void OnWindowResize(const glm::ivec2& oldSize, const glm::ivec2& newSize) {}

	/**
	 * Notifies the application layer that the application's config has been modified, and may require
	 * a reload or re-configuration of settings
	 * 
	 * @param config The updated application configuration settings for this layer
	 */
	virtual void NotifyAppConfigChanged(const nlohmann::json& config) {}

	/**
	 * Allows the layer to return default application configuration settings for the layer
	 * 
	 * @returns A JSON object containing the default configuration parameters for the layer
	 */
	virtual nlohmann::json GetDefaultConfig() { return {}; }

protected:
	ApplicationLayer() = default;
};