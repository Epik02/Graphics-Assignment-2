#include "MeshResource.h"
#include <filesystem>

#include "Utils/ObjLoader.h"

namespace Gameplay {
	MeshResource::MeshResource() :
		IResource(),
		Filename(""),
		MeshBuilderParams(std::vector<MeshBuilderParam>()),
		Mesh(nullptr),
		BulletTriMesh(nullptr)
	{ }

	MeshResource::MeshResource(const std::string& filename) :
		IResource(),
		Filename(filename),
		MeshBuilderParams(std::vector<MeshBuilderParam>()),
		Mesh(nullptr),
		BulletTriMesh(nullptr)
	{
		Mesh = ObjLoader::LoadFromFile(filename);
	}

	MeshResource::~MeshResource() = default;

	nlohmann::json MeshResource::ToJson() const {
		nlohmann::json result;
		if (MeshBuilderParams.size() > 0) {
			std::vector<nlohmann::json> params = std::vector<nlohmann::json>();
			params.resize(MeshBuilderParams.size());
			for (int ix = 0; ix < MeshBuilderParams.size(); ix++) {
				params[ix] = MeshBuilderParams[ix].ToJson();
			}
			result["params"] = params;
		} else {
			result["filename"] = Filename.empty() ? "null" : Filename;
		}
		return result;
	}

	MeshResource::Sptr MeshResource::FromJson(const nlohmann::json & blob)
	{
		MeshResource::Sptr result = std::make_shared<MeshResource>();
		if (blob.contains("params") && blob["params"].is_array()) {
			std::vector<nlohmann::json> meshbuilderParams = blob["params"].get<std::vector<nlohmann::json>>();
			MeshBuilder<VertexPosNormTexColTangents> mesh;
			for (int ix = 0; ix < meshbuilderParams.size(); ix++) {
				MeshBuilderParam p = MeshBuilderParam::FromJson(meshbuilderParams[ix]);
				result->MeshBuilderParams.push_back(p);
				MeshFactory::AddParameterized(mesh, p);
			}
			MeshFactory::CalculateTBN(mesh);
			result->Mesh = mesh.Bake();
		} else {
			result->Filename = JsonGet<std::string>(blob, "filename", "null");
			if (result->Filename != "null" && std::filesystem::exists(result->Filename)) {
				#ifdef OPTIMIZED_OBJ_LOADER
				result->Mesh = OptimizedObjLoader::LoadFromFile(result->Filename);
				#else
				result->Mesh = ObjLoader::LoadFromFile(result->Filename);
				#endif

			}
		}
		return result;
	}

	void MeshResource::GenerateMesh() {
		MeshBuilder<VertexPosNormTexColTangents> mesh;
		for (auto& param : MeshBuilderParams) {
			MeshFactory::AddParameterized(mesh, param);
		}
		MeshFactory::CalculateTBN(mesh);
		Mesh = mesh.Bake();
	}

	void MeshResource::AddParam(const MeshBuilderParam & param) {
		MeshBuilderParams.push_back(param);
	}
}