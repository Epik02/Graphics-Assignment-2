#pragma once
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/MeshResource.h"
#include "Gameplay/Material.h"
#include "Utils/MeshFactory.h"

/// <summary>
/// Provides information for a object to be rendered
/// 
/// Replaces the mesh and material info from RenderObject
/// </summary>
class RenderComponent : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<RenderComponent> Sptr;

	RenderComponent();
	RenderComponent(const Gameplay::MeshResource::Sptr& mesh, const Gameplay::Material::Sptr& material);

	/// <summary>
	/// Gets the mesh resource which contains the mesh and serialization info for
	/// this render component
	/// </summary>
	const Gameplay::MeshResource::Sptr& GetMeshResource() const;
	/// <summary>
	/// Gets the VAO of the underlying mesh resource
	/// </summary>
	VertexArrayObject::Sptr GetMesh() const;
	/// <summary>
	/// Gets the material that this renderer is using
	/// </summary>
	const Gameplay::Material::Sptr& GetMaterial() const;

	/// <summary>
	/// Sets this render component's mesh resource, from which the VAO will be retrieved for rendering
	/// </summary>
	/// <param name="mesh">The mesh resource containing info about the model to be rendered</param>
	RenderComponent* SetMesh(const Gameplay::MeshResource::Sptr& mesh);
	/// <summary>
	/// Sets this render component's material, which will be used to feed material parameters to the appropriate
	/// shader, and ensure that the shader is bound when this object should be drawn
	/// </summary>
	/// <param name="mat">The material for this object</param>
	RenderComponent* SetMaterial(const Gameplay::Material::Sptr& mat);

	// Inherited from IComponent

	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static RenderComponent::Sptr FromJson(const nlohmann::json& data);
	MAKE_TYPENAME(RenderComponent);

protected:
	// The object's mesh
	Gameplay::MeshResource::Sptr _mesh;
	// The object's material
	Gameplay::Material::Sptr      _material;

	// If we want to use MeshFactory, we can populate this list
	std::vector<MeshBuilderParam> _meshBuilderParams;
};