#pragma once
#include "../IEditorWindow.h"
#include "Gameplay/GameObject.h"

/**
 * Handles an editor window for rendering our game object hierarchy
 */
class HierarchyWindow : public IEditorWindow {
public:
	MAKE_PTRS(HierarchyWindow)

	HierarchyWindow();
	virtual ~HierarchyWindow();

	// Inherited from IEditorWindow

	virtual void Render() override;

protected:
	void _RenderObjectNode(Gameplay::GameObject::Sptr object, int depth = 0);
};