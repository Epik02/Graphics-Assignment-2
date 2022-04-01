#pragma once

#include <json.hpp>
#include <unordered_map>
#include <typeindex>

#include "Utils/GUID.hpp"
#include "Utils/ResourceManager/IResource.h"
#include "Utils/StringUtils.h"

/// <summary>
/// Utility class for managing and loading resources from JSON
/// manifest files
/// </summary>
class ResourceManager {
public:
	/// <summary>
	/// Initializes the resource manager and performs any first-time
	/// setup required
	/// </summary>
	static void Init();

	/// <summary>
	/// Creates a new asset, and forwards the arguments to it's constructor
	/// </summary>
	/// <typeparam name="T">The type of asset to create</typeparam>
	/// <typeparam name="...TArgs">The types for the arguments to forward to the constructor</typeparam>
	/// <param name="...args">The arguments to forward to the constructor</param>
	/// <returns>The GUID of the newly created asset</returns>
	template <typename T, typename ... TArgs, typename = std::enable_if<is_valid_resource<T>()>::type>
	static std::shared_ptr<T> CreateAsset(TArgs&&... args) {
		// Create and store the asset
		std::shared_ptr<T> asset = std::make_shared<T>(std::forward<TArgs>(args)...);
		_resources[std::type_index(typeid(T))][asset->IResource::GetGUID()] = asset;

		// Get the JSON representation of the asset so we can store it in the manifest
		nlohmann::json data = asset->ToJson();

		// Make sure the data has the GUID
		std::string guid = asset->IResource::GetGUID().str();
		data["guid"] = guid;

		// Store the JSON data in the resource manifest (based on the type's name)
		_manifest[StringTools::SanitizeClassName(typeid(T).name())][guid] = data;
		return asset;
	}

	/// <summary>
	/// Gets a shared pointer to the resource with the given type and GUID
	/// </summary>
	/// <typeparam name="T">The type of resource to retreive</typeparam>
	/// <param name="id">The ID of the resource to retrieve</param>
	/// <returns>The resource with the given GUID, or nullptr if none exists</returns>
	template<typename T, typename = std::enable_if<is_valid_resource<T>()>::type>
	static std::shared_ptr<T> Get(Guid id) {
		// Try and grab the asset from the resource pool
		std::shared_ptr<T> result =  std::dynamic_pointer_cast<T>(_resources[std::type_index(typeid(T))][id]);

		// If the asset is null, we can try finding it in the manifest to load it
		if (result == nullptr) {
			// Get the type name it'll be stored under
			std::string typeName = StringTools::SanitizeClassName(typeid(T).name());

			// If the manifest has an entry, we can load it!
			if (_manifest[typeName].contains(id)) {
				// Invoke the loader function with the manifest data
				_typeLoaders[typeName](_manifest[typeName][id]);

				// Search resources again to get the resource
				return std::dynamic_pointer_cast<T>(_resources[std::type_index(typeid(T))][id]);
			}
		}

		// If result wasn't null, or couldn't be found in the manifest, return here
		return result;
	}

	/// <summary>
	/// Registers a resource type with the resource manager, only types that have been registered
	/// can be loaded from JSON manifest files!
	/// </summary>
	/// <typeparam name="T">The type to register, must satisfy the is_valid_resource constraint</typeparam>
	/// <typeparam name=""></typeparam>
	template <typename T, typename = std::enable_if<is_valid_resource<T>()>::type>
	static void RegisterType() {
		// Extract the type name from a sanitized version of they typeid name
		std::string typeName = StringTools::SanitizeClassName(typeid(T).name());

		// Create the type loader for the type
		_typeLoaders[typeName] = [](const nlohmann::json& data) {
			IResource::Sptr res = T::FromJson(data);
			res->OverrideGUID(Guid(data["guid"]));
			_resources[std::type_index(typeid(T))][res->GetGUID()] = res;
			return res->GetGUID();
		};

		// Make sure we haven't registered the type yet, then add an empty object
		// to the manifest to ensure it can be saved
		if (!_manifest.contains(typeName)) {
			_manifest[typeName] = nlohmann::json();
		}
	}

	/// <summary>
	/// Iterates over all resources of the given type and invokes a method with them
	/// </summary>
	/// <typeparam name="ResourceType">The type of resource to iterate on</typeparam>
	/// <param name="callback">The callback to invoke with the components</param>
	/// <param name="includeDisabled">True to include disabled components, false if otherwise</param>
	template <
		typename ResourceType,
		typename = typename std::enable_if<std::is_base_of<IResource, ResourceType>::value>::type>
		static void Each(std::function<void(const std::shared_ptr<ResourceType>&)> callback, bool includeDisabled = false) {

		// We can use typeid and type_index to get a unique ID for our types
		std::type_index type = std::type_index(typeid(ResourceType));

		// Iterate over all the resources in the store
		for (auto& [key, value] : _resources[type]) {
			// If the pointer is alive and matches our enabled criteria, invoke the callback
			if (value != nullptr) {
				// Upcast to resource type and invoke the callback
				callback(std::dynamic_pointer_cast<ResourceType>(value));
			}
		}
	}

	/// <summary>
	/// Gets the current JSON manifest
	/// </summary>
	static const nlohmann::ordered_json& GetManifest();
	/// <summary>
	/// Loads a manifest file into the resource manager. Note that this will not perform load on the assets themselves 
	/// unless preloadAssets is set to true
	/// </summary>
	/// <param name="path">The path to the JSON manifest file</param>
	/// <param name="preloadAssets">True if all assets should be loaded into memory</param>
	static void LoadManifest(const std::string& path, bool preloadAssets = false);
	/// <summary>
	/// Saves the manifest to the given JSON file
	/// </summary>
	/// <param name="path">The path to the file to output</param>
	static void SaveManifest(const std::string& path);

	/// <summary>
	/// Releases all resources held by the resource manager
	/// </summary>
	static void Cleanup();

protected:
	/// <summary>
	/// This is a map of maps
	/// The top level map uses type_index, so there's a map per resource type
	/// The inner map handles mapping GUIDs to the corresponding resource
	/// </summary>
	static std::map<std::type_index, std::map<Guid, IResource::Sptr>> _resources;
	/// <summary>
	/// This map stores registered types, so we can load them from JSON files
	/// </summary>
	static std::map<std::string, std::function<Guid(const nlohmann::json&)>> _typeLoaders;

	/// <summary>
	/// We use an ORDERED JSON file to allow serializing types in the order they are registered.
	/// This allows us to register dependencies before the dependent resource
	/// </summary>
	static nlohmann::ordered_json _manifest;
};