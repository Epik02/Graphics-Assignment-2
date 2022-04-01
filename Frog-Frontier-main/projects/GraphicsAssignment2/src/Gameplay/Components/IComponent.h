#pragma once
#include <memory>
#include "json.hpp"
#include <imgui.h>
#include <GLM/glm.hpp>

#include "Utils/StringUtils.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ResourceManager/IResource.h"
#include "Utils/TypeHelpers.h"

namespace Gameplay {
	// We pre-declare GameObject to avoid circular dependencies in the headers
	class GameObject;

	namespace Physics {
		class TriggerVolume;
		class RigidBody;
	}

	/// <summary>
	/// Base class for components that can be attached to game objects
	/// 
	/// NOTE:
	/// Components must additionally define a static method as such:
	/// 
	/// static std::shared_ptr<Type> FromJson(const nlohmann::json&);
	/// 
	/// where Type is the Type of component
	/// </summary>
	class IComponent : public IResource {
	public:
		typedef std::shared_ptr<IComponent> Sptr;

		/// <summary>
		/// True when this component is enabled and should perform update and 
		/// renders
		/// </summary>
		bool IsEnabled;

		virtual ~IComponent();

		/// <summary>
		/// Invoked when a dynamic rigidbody attached to the parent gameobject has entered
		/// a trigger volume
		/// </summary>
		/// <param name="trigger"></param>
		virtual void OnEnteredTrigger(const std::shared_ptr<Physics::TriggerVolume>& trigger){};

		/// <summary>
		/// Invoked when a dynamic rigidbody attached to the parent gameobject has left
		/// a trigger volume
		/// </summary>
		/// <param name="trigger"></param>
		virtual void OnLeavingTrigger(const std::shared_ptr<Physics::TriggerVolume>& trigger){};

		/// <summary>
		/// Invoked when a dynamic rigidbody has entered a trigger attached to the same gameobject
		/// as this component
		/// </summary>
		/// <param name="body"></param>
		virtual void OnTriggerVolumeEntered(const std::shared_ptr<Physics::RigidBody>& body) {};
		/// <summary>
		/// Invoked when a dynamic rigidbody has left a trigger attached to the same gameobject
		/// as this component
		/// </summary>
		/// <param name="body"></param>
		virtual void OnTriggerVolumeLeaving(const std::shared_ptr<Physics::RigidBody>& body) {};

		/// <summary>
		/// Allows components to perform an action before child GUI items are rendered
		/// </summary>
		virtual void StartGUI() {}
		/// <summary>
		/// Allows components to render GUI elements to the screen
		/// </summary>
		virtual void RenderGUI() {}
		/// <summary>
		///  Allows components to perform an action after child GUI items are rendered
		/// </summary>
		virtual void FinishGUI() {}

		/// <summary>
		/// Invoked when a component has been added to a game object, note that this function
		/// should only perform local setup (i.e never look for game objects or other components)
		/// </summary>
		/// <param name="context">The game object that the component belongs to</param>
		virtual void OnLoad() { };
		/// <summary>
		/// Invoked when the scene has finished loading, and all objects and components
		/// are set up
		/// </summary>
		/// <param name="context">The game object that the component belongs to</param>
		virtual void Awake() { };

		/// <summary>
		/// Invoked during the update loop
		/// </summary>
		/// <param name="context">The game object that the component belongs to</param>
		/// <param name="deltaTime">The time since the last frame, in seconds</param>
		virtual void Update(float deltaTime) {};

		/// <summary>
		/// All components should override this to allow us to render component
		/// info in ImGui for easy editing
		/// </summary>
		/// <param name="context">The game object that the component belongs to</param>
		virtual void RenderImGui() = 0;

		/// <summary>
		/// Returns the component's type name
		/// To override in child classes, use MAKE_TYPENAME(Type) instead of
		/// manually implementing, otherwise serialization may have isues
		/// </summary>
		virtual std::string ComponentTypeName() const = 0;

		/// <summary>
		/// Gets the gameobject that this component is attached to
		/// </summary>
		GameObject* GetGameObject() const;

		/// <summary>
		/// Checks whether this component's gameobject has a component of the given type
		/// </summary>
		/// <typeparam name="T">The type of component to search for</typeparam>
		template <typename T, typename = typename std::enable_if<std::is_base_of<IComponent, T>::value>::type>
		bool HasComponent() {
			return _context->Has<T>();
		}

		/// <summary>
		/// Gets the component of the given type from the parent gameobject, or nullptr if it does not exist
		/// </summary>
		/// <typeparam name="T">The type of component to search for</typeparam>
		template <typename T, typename = typename std::enable_if<std::is_base_of<IComponent, T>::value>::type>
		std::shared_ptr<T> GetComponent() {
			return _context->Get<T>();
		}

		/// <summary>
		/// Adds a component of the given type to the parent gameobject. Note that only one component
		/// of a given type may be attached to a gameobject
		/// </summary>
		/// <typeparam name="T">The type of component to add</typeparam>
		/// <typeparam name="TArgs">The arguments to forward to the component constructor</typeparam>
		template <typename T, typename ... TArgs>
		std::shared_ptr<T> AddComponent(TArgs&&... args) {
			return _context->Add<T>(std::forward<TArgs>(args)...);
		}

		/// <summary>
		/// For passing in place of a raw pointer to other APIs like bullet
		/// </summary>
		std::weak_ptr<IComponent>& SelfRef();

	protected:
		IComponent();

	private:
		friend class ComponentManager;
		friend class GameObject;

		std::type_index _realType;
		GameObject* _context;

		// By storing a weak pointer to ourselves, we can pass a pointer to this
		// for things like bullet user pointers
		std::weak_ptr<IComponent> _weakSelfPtr;

		static void LoadBaseJson(const IComponent::Sptr& result, const nlohmann::json& blob);
		static void SaveBaseJson(const IComponent::Sptr& instance, nlohmann::json& data);
	};

	/// <summary>
	/// Returns true if the given type is a valid component type
	/// </summary>
	/// <typeparam name="T">The type to check</typeparam>
	template <typename T>
	constexpr bool is_valid_component() {
		return std::is_base_of<IComponent, T>::value && test_json<T, const nlohmann::json&>::value;
	}
}

// Defines the ComponentTypeName interface to match those used elsewhere by other systems
#define MAKE_TYPENAME(T) \
	inline virtual std::string ComponentTypeName() const { \
		static std::string name = StringTools::SanitizeClassName(typeid(T).name()); return name; }
