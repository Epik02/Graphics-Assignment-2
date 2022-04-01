#include "ImGuiDebugLayer.h"
#include "../Application.h"
#include "Utils/ImGuiHelper.h"
#include "imgui_internal.h"
#include "Gameplay/Scene.h"
#include "../Timing.h"
#include "Utils/Windows/FileDialogs.h"
#include <filesystem>
#include "RenderLayer.h"

#include "../Windows/HierarchyWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Windows/MaterialsWindow.h"
#include "../Windows/TextureWindow.h"
#include "../Windows/DebugWindow.h"
#include "../Windows/GBufferPreviews.h"
#include "../Windows/PostProcessingSettingsWindow.h"

#include "Graphics/DebugDraw.h"

ImGuiDebugLayer::ImGuiDebugLayer() :
	ApplicationLayer(),
	_dockInvalid(true)
{
	Name = "ImGui Debug Layer";
	Overrides = AppLayerFunctions::OnAppLoad | AppLayerFunctions::OnAppUnload | AppLayerFunctions::OnPreRender | AppLayerFunctions::OnRender | AppLayerFunctions::OnPostRender;
}

ImGuiDebugLayer::~ImGuiDebugLayer() = default;

void ImGuiDebugLayer::OnAppLoad(const nlohmann::json& config)
{
	Application& app = Application::Get();

	// Register our windows
	RegisterWindow<HierarchyWindow>();
	RegisterWindow<InspectorWindow>();
	RegisterWindow<MaterialsWindow>();
	RegisterWindow<TextureWindow>();
	RegisterWindow<DebugWindow>();
	RegisterWindow<GBufferPreviews>();
	RegisterWindow<PostProcessingSettingsWindow>();
}

void ImGuiDebugLayer::OnAppUnload()
{

}

void ImGuiDebugLayer::OnPreRender()
{

}

void ImGuiDebugLayer::OnRender(const Framebuffer::Sptr& prevLayer)
{
	using namespace Gameplay;
	Application& app = Application::Get();

	// We need to get the primary viewport, as well as track the ID of the primary window dock node
	glm::ivec2 size;
	ImGuiID dockspaceID;
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Only render if size of the window is non-zero
	if (viewport->Size.x > 0 && viewport->Size.y > 0) {
		// Set up where our primary dock node will be placed
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Set up the flags for the primary dock node
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		// Disable styles for the primary dock node
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0,0,0,0 });

		// Our primary dock window (which will be hidden)
		ImGui::Begin("Docker Window", nullptr, window_flags);
		
		// Pop styling variables so they don't effect other resources
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();

		// Get the ID of the main window dock
		ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
		dockspaceID = ImGui::GetID("MainDocker");

		// If our dock layout is invalid, or the node does not exist, recreate our dockspace
		if (_dockInvalid | !ImGui::DockBuilderGetNode(dockspaceID)) {
			// Remove the root node if it already exists, and then re-add it so we can clear any previous config
			ImGui::DockBuilderRemoveNode(dockspaceID);
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);

			// Make node fill the entire viewport
			ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

			// Create a copy of the dockspace ID that we can modify
			ImGuiID dock_main_id = dockspaceID;

			// Iterate over all the windows
			for (const auto& window : _windows) {
				// If the window is a child of the root node and is open
				if (window->ParentName.empty() && window->Open && window->SplitDirection != ImGuiDir_None) {
					// Split the main window depending on the window's direction and depth, store ID in window
					ImGuiID dockId = ImGui::DockBuilderSplitNode(dock_main_id, window->SplitDirection, window->SplitDepth, nullptr, &dock_main_id);
					window->DockId = dockId;
				}
			}

			// Iterate over all the windows again now that root nodes have been figured out
			for (const auto& window : _windows) {
				// If the window is a child of another window and is open
				if (!window->ParentName.empty() && window->Open) {

					// Determine the ID, split direction and depth based on what windows are open
					// If the parent is closed, the window will assume it's parent's position in the docking tree
					ImGuiDir direction = window->SplitDirection;
					float    dist = window->SplitDepth;
					ImGuiID& parentDock = _FindOpenParentWindow(window, dock_main_id, &direction, &dist);
					
					if (direction != ImGuiDir_None) {
						// Split the parent node, store the ID in the window
						ImGuiID dockId = ImGui::DockBuilderSplitNode(parentDock, direction, dist, nullptr, &parentDock);
						window->DockId = dockId;
					}
				}
			}

			// Iterate over all windows yet again and add all open windows to the dock window
			for (const auto& window : _windows) {
				if (window->Open && window->SplitDirection != ImGuiDir_None) {
					ImGui::DockBuilderDockWindow(window->Name.c_str(), window->DockId);
				}
			}

			// Add a specialized node for our game view
			ImGui::DockBuilderDockWindow("Game View", dock_main_id);
			ImGui::DockBuilderGetNode(dock_main_id)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

			// We're done building docking nodes!
			ImGui::DockBuilderFinish(dock_main_id);
		}

		// Make sure the primary dock node fills the window
		ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

		// Start the docking space
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), 0);

		// Start the menu bar at the top of the application
		if (ImGui::BeginMenuBar())
		{
			// Render the File menu
			if (ImGui::BeginMenu("File")) {
				// New scene item
				if (ImGui::MenuItem("New Scene", NULL, false)) {
					app.LoadScene(std::make_shared<Gameplay::Scene>());
				}

				// Load scene item
				if (ImGui::MenuItem("Load Scene", NULL, false)) {
					std::optional<std::string> path = FileDialogs::OpenFile("Scene File\0*.json\0\0");
					if (path.has_value()) {
						app.LoadScene(path.value());
					}
				}

				// Save scene item
				if (ImGui::MenuItem("Save Scene", NULL, false)) {
					std::optional<std::string> path = FileDialogs::SaveFile("Scene File\0*.json\0\0");
					if (path.has_value()) {
						app.CurrentScene()->Save(path.value());

						std::string newFilename = std::filesystem::path(path.value()).stem().string() + "-manifest.json";
						ResourceManager::SaveManifest(newFilename);
					}
				}

				ImGui::EndMenu();
			}

			for (const auto& window : _windows) {
				if (*(window->Requirements & EditorWindowRequirements::Menubar)) {
					if (ImGui::BeginMenu(window->Name.c_str())) {
						window->RenderMenuBar();
						ImGui::EndMenu();
					}
				}
			}

			// Windows menu
			if (ImGui::BeginMenu("Windows")) {
				// We want to try and minimize how often the menu closes
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

				// Render checkbox items for all the windows we have (to open and close them)
				for (const auto& window : _windows) {
					if (*(window->Requirements & EditorWindowRequirements::Window)) {
						ImGui::MenuItem(window->Name.c_str(), nullptr, &window->Open);
					}
				}
				ImGui::PopItemFlag();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		ImGui::End();

		_dockInvalid = false;

		// Handle rendering the primary game viewport
		_RenderGameWindow();

		// Iterate over all the windows
		for (const auto& window : _windows) {
			if (*(window->Requirements & EditorWindowRequirements::Window)) {
				// Track whether the window was previously open, only render open windows
				bool wasOpen = window->Open;
				if (window->Open) {
					bool open = ImGui::Begin(window->Name.c_str(), &window->Open, window->WindowFlags);

					// If the window was open or closed, mark our dock node as invalid
					if (open != wasOpen) {
						_dockInvalid = true;
					}

					// If window is closed, early bail and move to next window
					if (!open) {
						ImGui::End();
						continue;
					}

					// Otherwise render the window
					window->Render();
					ImGui::End();
				}
			}
		}
	}
}

void ImGuiDebugLayer::OnPostRender()
{
	// HACK HACK HACK - Getting debug gizmos to show up
	Application& app = Application::Get();
	const glm::uvec4& viewport = app.GetPrimaryViewport();
	glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
 
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);

	glClear(GL_DEPTH_BUFFER_BIT);

	DebugDrawer::Get().SetViewProjection(app.CurrentScene()->MainCamera->GetViewProjection());
	DebugDrawer::Get().FlushAll();
}

void ImGuiDebugLayer::_RenderGameWindow()
{
	using namespace Gameplay;
	Application& app = Application::Get();

	// Setting up the style and window flags for the game viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0,0,0,0 });
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoTitleBar;

	// We need the application viewport so we can figure out where the game window is relative to it
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 rootPos = viewport->Pos;
	ImVec2 rootSize = viewport->Size;

	// Start the window, we can also let the app know whether it has focus by checking the ImGui window's focus
	ImGui::Begin("Game View", nullptr, window_flags);
	ImGui::PopStyleVar(3);
	app.IsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	// Grab the current scene the application is displaying
	Scene::Sptr scene = app.CurrentScene();

	// Janky ass button text for the play/stop button
	static char buffer[64];
	sprintf_s(buffer, "%s###PLAY_STOP", scene->IsPlaying ? "[]" : ">");

	// Remove spacing around buttons
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	// Draw the play/stop button
	if (ImGui::Button(buffer)) {
		// Save scene so it can be restored when exiting play mode
		if (!scene->IsPlaying) {
			_backupState = scene->ToJson();
		}

		// Toggle state
		scene->IsPlaying = !scene->IsPlaying;

		// If we've gone from playing to not playing, restore the state from before we started playing
		if (!scene->IsPlaying) {
			scene = nullptr;
			// We reload to scene from our cached state
			scene = Scene::FromJson(_backupState);
			app.LoadScene(scene);
		}
	}

	ImGui::SameLine();

	// Draw pause button
	if (ImGui::Button("||")) {
		float newScale = Timing::TimeScale();
		newScale = newScale > 1 ? 0 : 1;
		Timing::SetTimeScale(newScale);
	}

	ImGui::SameLine();

	ImGui::PopStyleVar();

	ImGui::Text("FPS: %.*0f", 3, 1.0f / Timing::Current().DeltaTime());

	// Determine the relative position of the window
	ImVec2 subPos = ImGui::GetWindowPos();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 relPos ={ subPos.x - rootPos.x + cursorPos.x, subPos.y - rootPos.y + cursorPos.y };

	// We use a local static to track changes in window size
	static ImVec2 prevSize = ImVec2(0.0f, 0.0f);
	ImVec2 size = ImGui::GetContentRegionAvail();

	// If our ImGui window has been resized, let the application know
	if (size.x != prevSize.x || size.y != prevSize.y) {
		app.ResizeWindow({ size.x, size.y });
		prevSize = size;
	}

	// Tell the viewport where to render the game contents to
	app.SetPrimaryViewport(glm::vec4(relPos.x, rootSize.y - relPos.y - size.y, size.x, size.y));
	
	// Finish window
	ImGui::End();

	// Pop all the styling changes
	ImGui::PopStyleColor();
}

ImGuiID& ImGuiDebugLayer::_FindOpenParentWindow(const IEditorWindow::Sptr& window, ImGuiID& mainID, ImGuiDir* direction, float* dist)
{
	// Find the parent window 
	auto it = std::find_if(_windows.begin(), _windows.end(), [&](const auto& win) { return win->Name == window->ParentName; });

	// If parent exists
	if (it != _windows.end()) {
		// If parent is open, return it's ID and do not modify splitting parameters
		if ((*it)->Open) {
			return (*it)->DockId;
		}
		// Parent is not open
		else {
			// Inherit the splitting parameters from the parent
			if (direction != nullptr) *direction = (*it)->SplitDirection;
			if (dist != nullptr) *dist = (*it)->SplitDepth;

			// Recursively search for an open parent in the tree
			return _FindOpenParentWindow(*it, mainID, direction, dist);
		}
	} 
	// Parent does not exist, return the main docking node
	else {
		return mainID;
	}
}
