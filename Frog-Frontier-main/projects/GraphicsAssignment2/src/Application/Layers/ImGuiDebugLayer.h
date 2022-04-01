#pragma once
#include "../ApplicationLayer.h"
#include "Gameplay/Physics/BulletDebugDraw.h"
#include "../IEditorWindow.h"
#include "Logging.h"

/**
 * The ImGui Debug Layer allows us to handle editor and debug windows using ImGUI
 */
class ImGuiDebugLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(ImGuiDebugLayer)

	ImGuiDebugLayer();
	virtual ~ImGuiDebugLayer();

	template <typename T, typename = typename std::enable_if<std::is_base_of<IEditorWindow, T>::value>::type>
	void RegisterWindow() {
		// Iterate over all the pointers in the layers list
		for (const auto& ptr : _windows) {
			// If the pointer type matches T, we return that layer, making sure to cast it back to the requested type
			if (std::type_index(typeid(*ptr.get())) == std::type_index(typeid(T))) {
				LOG_WARN("This window has already been registered!");
				return;
			}
		}
		_windows.push_back(std::make_shared<T>());
	}

	template <typename T, typename = typename std::enable_if<std::is_base_of<IEditorWindow, T>::value>::type>
	std::shared_ptr<T> GetWindow() {
		// Iterate over all the pointers in the layers list
		for (const auto& ptr : _windows) {
			// If the pointer type matches T, we return that layer, making sure to cast it back to the requested type
			if (std::type_index(typeid(*ptr.get())) == std::type_index(typeid(T))) {
				return std::dynamic_pointer_cast<T>(ptr);
			}
		}
		return nullptr;
	}

	// Inherited from ApplicationLayer

	virtual void OnAppLoad(const nlohmann::json& config) override;
	virtual void OnAppUnload() override;
	virtual void OnPreRender() override;
	virtual void OnRender(const Framebuffer::Sptr& prevLayer) override;
	virtual void OnPostRender() override;

protected:
	std::vector<IEditorWindow::Sptr> _windows;
	nlohmann::json _backupState;
	bool           _dockInvalid;

	void _RenderGameWindow();
	ImGuiID& _FindOpenParentWindow(const IEditorWindow::Sptr& window, ImGuiID& mainID, ImGuiDir* direction, float* dist);
};