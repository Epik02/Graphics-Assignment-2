#pragma once
#include <functional>
#include "IComponent.h"
#include <typeindex>
#include <optional>
#include <Logging.h>

namespace Gameplay {
	/// <summary>
	/// Helper class for component types, this class is what lets us load component types
	/// from scene files, as well as providing a way to iterate over all active components
	/// of a given type (and sort them in the future!)
	/// </summary>
	class ComponentManager {
	public:
		typedef std::function<IComponent::Sptr(const nlohmann::json&)> LoadComponentFunc;
		typedef std::function<IComponent::Sptr()> CreateComponentFunc;

		inline void Clear() {
			_Components.clear();
		}

		/// <summary>
		/// Loads a component with the given type name from a JSON blob
		/// If the type name does not correspond to a registered type, will
		/// return nullptr
		/// </summary>
		/// <param name="typeName">The name of the type to load (taken from GetComponentTypeName of component)</param>
		/// <param name="blob">The JSON blob to decode</param>
		/// <returns>The component as decoded from the JSON data, or nullptr</returns>
		inline IComponent::Sptr Load(const std::string& typeName, const nlohmann::json& blob) {
			// Try and get the type index from the name
			std::optional<std::type_index> typeIndex = _TypeNameMap[typeName];

			// If we have a value for type index, this component type was registered!
			if (typeIndex.has_value()) {
				// Get the load callback and make sure it exists
				LoadComponentFunc callback = _TypeLoadRegistry[typeIndex.value()];
				if (callback) {
					// Invoke the loader, also load additional component data
					IComponent::Sptr result = callback(blob);
					IComponent::LoadBaseJson(result, blob);

					// Make sure the component knows it's own type
					result->_realType = typeIndex.value();
					result->_weakSelfPtr = result;

					// Add the component to the global pools
					_Components[result->_realType].push_back(result);
					return result;
				}
			}
			return nullptr;
		}

		/// <summary>
		/// Creates a component with the given type name
		/// If the type name does not correspond to a registered type, will
		/// return nullptr
		/// </summary>
		/// <param name="typeName">The name of the type to load (taken from GetComponentTypeName of component)</param>
		/// <returns>A new component of the given type, or nullptr</returns>
		inline IComponent::Sptr Create(const std::string& typeName) {
			// Try and get the type index from the name
			std::optional<std::type_index> typeIndex = _TypeNameMap[typeName];

			// If we have a value for type index, this component type was registered!
			if (typeIndex.has_value()) {
				// Get the load callback and make sure it exists
				CreateComponentFunc callback = _TypeCreateRegistry[typeIndex.value()];
				if (callback) {
					// Invoke the loader, also load additional component data
					IComponent::Sptr result = callback();
					// Make sure the component knows it's own type
					result->_realType = typeIndex.value();
					result->_weakSelfPtr = result;
					// Add the component to the global pools
					_Components[result->_realType].push_back(result);
					return result;
				}
			}
			return nullptr;
		}

		/// <summary>
		/// Creates a component with the given type name
		/// If the type name does not correspond to a registered type, will
		/// return nullptr
		/// </summary>
		/// <param name="typeName">The name of the type to load (taken from GetComponentTypeName of component)</param>
		/// <returns>A new component of the given type, or nullptr</returns>
		inline IComponent::Sptr Create(const std::type_index& type) {
			// Try and get the type index from the name
			LOG_ASSERT(_TypeLoadRegistry[type] != nullptr, "You must register component types before creating them!");

			// Get the load callback and make sure it exists
			CreateComponentFunc callback = _TypeCreateRegistry[type];
			if (callback) {
				// Invoke the loader, also load additional component data
				IComponent::Sptr result = callback();
				// Make sure the component knows it's own type
				result->_realType = type;
				result->_weakSelfPtr = result;
				// Add the component to the global pools
				_Components[result->_realType].push_back(result);
				return result;
			}
			return nullptr;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="callback"></param>
		inline void EachType(std::function<void(const std::string& typeName, std::type_index type)> callback) {
			for (auto& [name, type] : _TypeNameMap) {
				if (type.has_value()) {
					callback(name, type.value());
				}
			}
		}

		/// <summary>
		/// Creates a new component and adds it to the global component pools
		/// </summary>
		/// <typeparam name="ComponentType">Type type of component to create</typeparam>
		/// <typeparam name="...TArgs">The types of params to forward to the component's constructor</typeparam>
		/// <param name="...args">The arguments to forward to the constructor</param>
		/// <returns>The new component that has been created</returns>
		template <
			typename ComponentType, 
			typename ... TArgs, 
			typename = typename std::enable_if<std::is_base_of<IComponent, ComponentType>::value>::type>
		std::shared_ptr<ComponentType> Create(TArgs&& ... args) {
			std::type_index type = std::type_index(typeid(ComponentType));
			LOG_ASSERT(_TypeLoadRegistry[type] != nullptr, "You must register component types before creating them!");

			// Create component, forwarding arguments
			std::shared_ptr<ComponentType> component = std::make_shared<ComponentType>(std::forward<TArgs>(args)...);

			// Make sure the component knows it's concrete type
			component->_realType = type;
			// Give the component a weak pointer to itself that it can upcast to a shared pointer when needed
			component->_weakSelfPtr = component;

			// Add to global component list for that type
			_Components[type].push_back(component);

			// Return the result
			return component;
		}

		/// <summary>
		/// Searches for a component with the given GUID, allowing components to cross reference each other
		/// and survive scene serialization
		/// </summary>
		/// <typeparam name="ComponentType">The type of component to get</typeparam>
		/// <param name="id">The unique ID of the component to get</param>
		/// <returns>The component with the given ID, or nullptr if it does not exist</returns>
		template <
			typename ComponentType,
			typename = typename std::enable_if<std::is_base_of<IComponent, ComponentType>::value>::type>
		std::shared_ptr<ComponentType> GetComponentByGUID(Guid id) {
			// We can use typeid and type_index to get a unique ID for our types
			std::type_index type = std::type_index(typeid(ComponentType));
			LOG_ASSERT(_TypeLoadRegistry[type] != nullptr, "You must register component types before creating them!");

			// Clear any dead weak pointers
			auto removeIt = std::remove_if(_Components[type].begin(), _Components[type].end(), [](const std::weak_ptr<IComponent>& ptr) {
				return ptr.expired();
			});
			_Components[type].erase(removeIt, _Components[type].end());

			// Search the component store for a component that matches that ID
			auto& it = std::find_if(_Components[type].begin(), _Components[type].end(), [&](const std::weak_ptr<IComponent>& ptr) {
				return (ptr.lock())->GetGUID() == id;
			});

			// If the component was found, return it. Otherwise return nullptr
			if (it != _Components[type].end()) {
				// We need to lock the weak pointer to convert it to a shared ptr
				return std::dynamic_pointer_cast<ComponentType>((*it).lock());
			} else {
				return nullptr;
			}
		}

		/// <summary>
		/// Iterates over all components of the given type and invokes a method with them
		/// </summary>
		/// <typeparam name="ComponentType">The type of component to iterate on</typeparam>
		/// <param name="callback">The callback to invoke with the components</param>
		/// <param name="includeDisabled">True to include disabled components, false if otherwise</param>
		template <
			typename ComponentType,
			typename = typename std::enable_if<std::is_base_of<IComponent, ComponentType>::value>::type>
		void Each(std::function<void(const std::shared_ptr<ComponentType>&)> callback, bool includeDisabled = false) {
			// We can use typeid and type_index to get a unique ID for our types
			std::type_index type = std::type_index(typeid(ComponentType));
			LOG_ASSERT(_TypeLoadRegistry[type] != nullptr, "You must register component types before creating them!");

			// Iterate over all the components in the store
			for (auto& wptr : _Components[type]) {
				// Lock the weak pointer to get a shared pointer (maybe)
				std::shared_ptr<IComponent> sptr = wptr.lock();
				// If the pointer is alive and matches our enabled criteria, invoke the callback
				if (sptr && sptr->IsEnabled | includeDisabled) {
					// Upcast to component type and invoke the callback
					callback(std::dynamic_pointer_cast<ComponentType>(sptr));
				}
			}
		}

		/// <summary>
		/// Attempts to register a given type as a component, should be called for each component type 
		/// at the start of you application
		/// </summary>
		/// <typeparam name="T">The type to register, should extend the IComponent interface and have appropriate static methods</typeparam>
		template <typename T>
		static void RegisterType() {
			// Make sure the component type is valid (see bottom of IComponent.h)
			static_assert(is_valid_component<T>(), "Type is not a valid component type!");

			// We use the type ID to map types to the underlying helpers
			std::type_index type(typeid(T));

			// if type NOT registered
			if (_TypeLoadRegistry.find(type) == _TypeLoadRegistry.end()) {
				// Store the loading function in the registry, as well as the
				// name to type index mapping
				_TypeLoadRegistry[type] = &ComponentManager::ParseTypeFromBlob<T>;
				_TypeCreateRegistry[type] = &ComponentManager::_InternalCreate<T>;
				_TypeNameMap[StringTools::SanitizeClassName(typeid(T).name())] = type;
			}
		}

		/// <summary>
		/// Removes all components of all types from the registry, whether they are referenced elsewhere or not
		/// </summary>
		inline void FlushAll() {
			_Components = std::unordered_map<std::type_index, std::vector<std::weak_ptr<IComponent>>>();
		}

	private:
		// Give component friend access so it can call Remove
		friend class IComponent;

		// This maps a readable type name to it's type_index. We use optional in case we try and access
		// an element that does not have a type (and unordered_map requires a default constructor, which
		// std::type_index does not have)
		inline static std::unordered_map<std::string, std::optional<std::type_index>> _TypeNameMap;
		// Stores functions to load components from JSON, indexed on the type that they load
		inline static std::unordered_map<std::type_index, LoadComponentFunc> _TypeLoadRegistry;
		// Stores functions to load components from JSON, indexed on the type that they load
		inline static std::unordered_map<std::type_index, CreateComponentFunc> _TypeCreateRegistry;

		// Weak pointers let us store a reference to an object stored by a shared pointer, without
		// actually increasing the reference count. Thus components will be destroyed at the correct
		// time (when the only reference is the one stored here).
		std::unordered_map<std::type_index, std::vector<std::weak_ptr<IComponent>>> _Components;  

		template <typename T>
		static IComponent::Sptr ParseTypeFromBlob(const nlohmann::json& blob) {
			return T::FromJson(blob);
		}

		template <typename ComponentType>
		static IComponent::Sptr _InternalCreate() {
			// We can use typeid and type_index to get a unique ID for our types
			std::type_index type = std::type_index(typeid(ComponentType));
			LOG_ASSERT(_TypeLoadRegistry[type] != nullptr, "You must register component types before creating them!");

			// Create component, forwarding arguments
			std::shared_ptr<ComponentType> component = std::make_shared<ComponentType>();

			// Make sure the component knows it's concrete type
			component->_realType = type;
			// Give the component a weak pointer to itself that it can upcast to a shared pointer when needed
			component->_weakSelfPtr = component;

			// Return the result
			return component;
		}

		/// <summary>
		/// Removes a given component from the global pools. To be used in the IComponent destructor
		/// </summary>
		/// <typeparam name="ComponentType">The type of component to destroy</typeparam>
		/// <param name="component">A raw pointer to the component to remove (should be called from IComponent destructor)</param>
		/// <returns>True if the element was removed, false if not</returns>
		inline void Remove(const IComponent* component) {
			if (_Components.size() == 0) return;

			// Make sure the component's type was one that was registered
			LOG_ASSERT(_TypeLoadRegistry[component->_realType] != nullptr, "You must register component types before creating them!");

			
			// Get a reference to the vector of components for easy access
			std::vector<std::weak_ptr<IComponent>>& componentStore = _Components[component->_realType];

			// Clear any dead weak pointers
			auto it = std::remove_if(componentStore.begin(), componentStore.end(), [](const std::weak_ptr<IComponent>& ptr) {
				return ptr.expired();
				});
			if (it != componentStore.end()) {
				componentStore.erase(it);
			}
		}
	};
}