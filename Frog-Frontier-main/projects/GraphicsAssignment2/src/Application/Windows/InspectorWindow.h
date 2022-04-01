#pragma once
#include "../IEditorWindow.h"
#include "Gameplay/Components/IComponent.h"

/**
 * Handles displaying editor information about game objects and their components
 */
class InspectorWindow final : public IEditorWindow {
public:
	MAKE_PTRS(InspectorWindow);
	InspectorWindow();
	virtual ~InspectorWindow();

	// Inherited from IEditorWindow

	virtual void Render() override;

protected:
	bool _RenderComponent(Gameplay::IComponent::Sptr component);
};