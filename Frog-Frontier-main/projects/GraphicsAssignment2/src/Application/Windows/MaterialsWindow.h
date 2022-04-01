#pragma once
#include "Application/IEditorWindow.h"
#include "Gameplay/Material.h"

/**
 * Handles displaying editor information about game objects and their components
 */
class MaterialsWindow final : public IEditorWindow {
public:
	MAKE_PTRS(MaterialsWindow);
	MaterialsWindow();
	virtual ~MaterialsWindow();

	// Inherited from IEditorWindow

	virtual void Render() override;

protected:
	void _RenderMaterial(Gameplay::Material::Sptr component);
};