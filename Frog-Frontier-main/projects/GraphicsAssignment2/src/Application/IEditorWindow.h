#pragma once

#include "Utils/Macros.h"
#include "imgui.h"
#include <string>
#include <EnumToString.h>

ENUM_FLAGS(EditorWindowRequirements, uint32_t,
	None = 0,
	Window = 1,
	Menubar = 2
);

/**
 * The editor window interface allows us to separate implementation for 
 * editor or debug windows
 */
class IEditorWindow {
public:
	MAKE_PTRS(IEditorWindow);
	NO_COPY(IEditorWindow);
	NO_MOVE(IEditorWindow);

	EditorWindowRequirements Requirements = EditorWindowRequirements::Window;
	bool        Open = true;
	ImGuiDir_   SplitDirection = ImGuiDir_::ImGuiDir_Left;
	float       SplitDepth = 0.2f;
	std::string Name;
	std::string ParentName;
	ImGuiID     DockId;
	ImGuiWindowFlags_ WindowFlags;

	virtual ~IEditorWindow() =default;

	virtual void Render() { };
	virtual void RenderMenuBar() { }

protected:
	IEditorWindow() = default;
};