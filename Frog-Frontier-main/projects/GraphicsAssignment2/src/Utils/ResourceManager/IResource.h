#pragma once
#include "Utils/GUID.hpp"
#include "json.hpp"

#include "Utils/TypeHelpers.h"

/// <summary>
/// Base class for graphics that the resource manager may want to manage
/// (ex: textures, models, shaders, materials, etc...)
/// 
/// NOTE:
/// Resources must additionally define a static method as such:
/// static std::shared_ptr<Type> FromJson(const nlohmann::json&);
/// where Type is the Type of resource
/// </summary>
class IResource {
public:
	typedef std::shared_ptr<IResource> Sptr;
	typedef std::weak_ptr<IResource>   Wptr;

	virtual ~IResource() = default;

	/// <summary>
	/// Gets the unique ID of this resource
	/// </summary>
	Guid GetGUID() const { return _guid; }
	/// <summary>
	/// Overrides the GUID of this resource with a new value
	/// Only use this if you know what you're doing!
	/// </summary>
	/// <param name="newValue">The new GUID for the object</param>
	void OverrideGUID(Guid newValue) { _guid = newValue; }

	virtual void ResolveReferences() {};

	/// <summary>
	/// Converts this resource into it's JSON manifest format
	/// Should contain all the data required to reconstruct the
	/// resource via the LoadFromJson command
	/// </summary>
	/// <returns>The JSON blob for the resource</returns>
	virtual nlohmann::json ToJson() const = 0;

protected:
	Guid _guid;
	IResource() : _guid(Guid::New()){}
};

/// <summary>
/// Returns true if the given type is a valid resource type
/// IE it needs to extend from IResource and implement a static
/// FromJson method
/// </summary>
/// <typeparam name="T">The type to check</typeparam>
template <typename T>
constexpr bool is_valid_resource() {
	return std::is_base_of<IResource, T>::value && test_json<T, const nlohmann::json&>::value;
}