#pragma once
#include "../IEditorWindow.h"
#include "Gameplay/GameObject.h"
#include "Graphics/Textures/Texture2D.h"

struct ImDrawList;

/**
 * Handles an editor window for rendering our game object hierarchy
 */
class GBufferPreviews : public IEditorWindow {
public:
	MAKE_PTRS(GBufferPreviews)

	GBufferPreviews();
	virtual ~GBufferPreviews();

	// Inherited from IEditorWindow

	virtual void Render() override; 

protected:
	void _RenderTexture2D(const Texture2D::Sptr& value, const ImVec2& size, const char* name);
};