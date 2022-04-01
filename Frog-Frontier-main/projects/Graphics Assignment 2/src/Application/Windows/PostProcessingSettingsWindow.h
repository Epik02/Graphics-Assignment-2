#pragma once
#include "../IEditorWindow.h"
#include "../Layers/PostProcessingLayer.h"

/**
 * Handles an editor window for rendering our game object hierarchy
 */
class PostProcessingSettingsWindow : public IEditorWindow {
public:
	MAKE_PTRS(PostProcessingSettingsWindow);

	PostProcessingSettingsWindow();
	virtual ~PostProcessingSettingsWindow();

	// Inherited from IEditorWindow

	virtual void Render() override;

protected:
	void _RenderEffect(const PostProcessingLayer::Effect::Sptr& value);
};