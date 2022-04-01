#pragma once
#include "Application/IEditorWindow.h"

/**
 * Handles displaying debug information
 */
class DebugWindow final : public IEditorWindow {
public:
	MAKE_PTRS(DebugWindow);
	DebugWindow();
	virtual ~DebugWindow();

	// Inherited from IEditorWindow

	virtual void RenderMenuBar() override;

protected:
};