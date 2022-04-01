#include "Gameplay/Components/RenderComponent.h"

#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ImGuiHelper.h"


RenderComponent::RenderComponent(const Gameplay::MeshResource::Sptr& mesh, const Gameplay::Material::Sptr& material) :
	_mesh(mesh), 
	_material(material), 
	_meshBuilderParams(std::vector<MeshBuilderParam>()) 
{ }

RenderComponent::RenderComponent() : 
	_mesh(nullptr), 
	_material(nullptr), 
	_meshBuilderParams(std::vector<MeshBuilderParam>())
{ }

RenderComponent* RenderComponent::SetMesh(const Gameplay::MeshResource::Sptr& mesh) {
	_mesh = mesh;
	return this;
}

const Gameplay::MeshResource::Sptr& RenderComponent::GetMeshResource() const {
	return _mesh;
}

VertexArrayObject::Sptr RenderComponent::GetMesh() const {
	return _mesh ? _mesh->Mesh : nullptr;
}

RenderComponent* RenderComponent::SetMaterial(const Gameplay::Material::Sptr& mat) {
	_material = mat;
	return this;
}

const Gameplay::Material::Sptr& RenderComponent::GetMaterial() const {
	return _material;
}

nlohmann::json RenderComponent::ToJson() const {
	nlohmann::json result;
	result["mesh"] = _mesh ? _mesh->GetGUID().str() : "null";
	result["material"] = _material ? _material->GetGUID().str() : "null";
	return result;
}

RenderComponent::Sptr RenderComponent::FromJson(const nlohmann::json& data) {
	RenderComponent::Sptr result = std::make_shared<RenderComponent>();
	result->_mesh = ResourceManager::Get<Gameplay::MeshResource>(Guid(data["mesh"].get<std::string>()));
	result->_material = ResourceManager::Get<Gameplay::Material>(Guid(data["material"].get<std::string>()));

	return result;
}

void RenderComponent::RenderImGui() {
	ImGui::Text("Indexed:   %s", GetMesh() != nullptr ? (_mesh->Mesh->GetIndexBuffer() != nullptr ? "true" : "false") : "N/A");
	ImGui::Text("Triangles: %d", GetMesh() != nullptr ? (_mesh->Mesh->GetElementCount() / 3) : 0);
	ImGui::Text("Source:    %s", (_mesh == nullptr || _mesh->Filename.empty()) ? "Generated" : _mesh->Filename.c_str());
	ImGui::Separator();
	ImGui::Text("Material:  %s", _material != nullptr ? _material->Name.c_str() : "NULL");
	ImGuiHelper::ResourceDragTarget<Gameplay::Material>(_material);
}
