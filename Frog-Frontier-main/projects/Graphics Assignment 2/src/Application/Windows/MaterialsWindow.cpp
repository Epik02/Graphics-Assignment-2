#include "MaterialsWindow.h"
#include "Utils/ResourceManager/ResourceManager.h"


MaterialsWindow::MaterialsWindow() :
	IEditorWindow()
{
	Name = "Materials";
	SplitDirection = ImGuiDir_::ImGuiDir_Down;
	SplitDepth = 0.2f;
}

MaterialsWindow::~MaterialsWindow() = default;

void MaterialsWindow::Render() {
	ResourceManager::Each<Gameplay::Material>([&](const Gameplay::Material::Sptr& material) {
		_RenderMaterial(material);
	});  
}

void MaterialsWindow::_RenderMaterial(Gameplay::Material::Sptr material) {
	material->RenderImGui();
}

