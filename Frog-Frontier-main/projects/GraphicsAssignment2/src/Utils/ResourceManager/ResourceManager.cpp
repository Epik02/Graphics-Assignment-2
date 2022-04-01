#include "Utils/ResourceManager/ResourceManager.h"

#include "Utils/ObjLoader.h"
#include "Utils/FileHelpers.h"
#include "Utils/StringUtils.h"

std::map<std::type_index, std::map<Guid, IResource::Sptr>> ResourceManager::_resources;
std::map<std::string, std::function<Guid(const nlohmann::json&)>> ResourceManager::_typeLoaders;

nlohmann::ordered_json ResourceManager::_manifest;

void ResourceManager::Init() {
	// TODO: initialize the resource manager once it's a bit more complex
	//_manifest["textures"]  = std::vector<nlohmann::json>();
	//_manifest["meshes"]    = std::vector<nlohmann::json>();
	//_manifest["shaders"]   = std::vector<nlohmann::json>();
	//_manifest["materials"] = std::vector<nlohmann::json>();
}

const nlohmann::ordered_json& ResourceManager::GetManifest() {
	return _manifest;
}

void ResourceManager::LoadManifest(const std::string& path, bool preloadAssets) {
	std::string contents = FileHelpers::ReadFile(path);
	nlohmann::ordered_json blob = nlohmann::ordered_json::parse(contents);
	_manifest = blob;

	if (preloadAssets) {
		for (auto& [typeName, items] : blob.items()) {
			auto& func = _typeLoaders[typeName];
			if (func) {
				for (auto& [guid, blob] : items.items()) {
					func(blob);
				}
			}
		}
	}
}

void ResourceManager::SaveManifest(const std::string& path) {
	// Update all resources in the manifest so they match their current representation
	for (auto& [type, map] : _resources) {
		std::string typeName = StringTools::SanitizeClassName(type.name());
		for (auto& [guid, res] : map) {
			if (res != nullptr) {
				_manifest[typeName][guid.str()] = res->ToJson();
				_manifest[typeName][guid.str()]["guid"] = res->GetGUID().str();
			}
		}
	}
	FileHelpers::WriteContentsToFile(path, _manifest.dump(1,'\t'));
}

void ResourceManager::Cleanup() {
	for (auto& [type, map] : _resources) {
		map.clear();
	}
}

