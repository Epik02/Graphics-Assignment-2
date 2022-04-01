#pragma once
#include "Application/IEditorWindow.h"
#include "Gameplay/Material.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Graphics/Textures/Texture3D.h"

/**
 * Handles displaying editor information about game objects and their components
 */
class TextureWindow final : public IEditorWindow {
public:
	MAKE_PTRS(TextureWindow);
	TextureWindow();
	virtual ~TextureWindow();

	// Inherited from IEditorWindow

	virtual void Render() override;

protected:
	void _RenderTexture3D(const Texture1D::Sptr& value, int width);
	void _RenderTexture2D(const Texture2D::Sptr& value, int width);
	void _RenderTexture1D(const Texture3D::Sptr& value, int width);
};
