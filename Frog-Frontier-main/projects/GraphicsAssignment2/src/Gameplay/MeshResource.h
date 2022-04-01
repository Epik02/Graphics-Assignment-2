#pragma once
#include "Utils/ResourceManager/IResource.h"
#include "Graphics/VertexArrayObject.h"
#include "Utils/MeshFactory.h"

// bullet triangle mesh pre-declaration
class btTriangleMesh;

namespace Gameplay {
	/// <summary>
	/// A mesh resource contains information on how to generate a VAO at runtime
	/// It can either load a VAO from a file, or generate one using the mesh 
	/// factory and MeshBuilderParams
	/// </summary>
	class MeshResource : public IResource {
	public:
		typedef std::shared_ptr<MeshResource> Sptr;

		// Default constructor
		MeshResource();
		/// <summary>
		/// Constructor for loading from file
		/// </summary>
		/// <param name="filename"></param>
		MeshResource(const std::string& filename);

		virtual ~MeshResource();

		/// <summary>
		/// The path that the mesh was loaded from, or empty to indicate that it was generated
		/// note that this will override any mesh builder params
		/// </summary>
		std::string                     Filename;
		/// <summary>
		/// The mesh builder parameters if this mesh resource is created at runtime
		/// </summary>
		std::vector<MeshBuilderParam>   MeshBuilderParams;

		/// <summary>
		/// The VAO for rendering this mesh in OpenGL
		/// </summary>
		VertexArrayObject::Sptr         Mesh;


		/// <summary>
		/// The optional mesh resource for generating colliders from this mesh
		/// </summary>
		MeshResource::Sptr             ColliderMeshData;
		/// <summary>
		/// Allows for bullet to generate a triangle mesh from this mesh and cache it
		/// </summary>
		std::shared_ptr<btTriangleMesh> BulletTriMesh;

		/// <summary>
		/// Generates a new mesh from the mesh builder parameters
		/// </summary>
		void GenerateMesh();
		/// <summary>
		/// Adds a new mesh builder parameter to the mesh
		/// </summary>
		/// <param name="param">The parameter to add</param>
		void AddParam(const MeshBuilderParam& param);

		// Inherited from IResource

		virtual nlohmann::json ToJson() const override;
		static MeshResource::Sptr FromJson(const nlohmann::json& blob);
	};
}