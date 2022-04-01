#pragma once
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Gameplay/Components/Camera.h"
#include "Gameplay/GameObject.h"

#include "Physics/BulletDebugDraw.h"

#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/Textures/Texture3D.h"

struct GLFWwindow;

class TextureCube;
class ShaderProgram;

class InspectorWindow;
class HierarchyWindow;

namespace Gameplay {
	namespace Physics {
		class RigidBody;
	}

	class MeshResource;
	class Material;

	/// <summary>
	/// Main class for our game structure
	/// Stores game objects, lights, the camera,
	/// and other top level state for our game
	/// </summary>
	class Scene {
	public:
		typedef std::shared_ptr<Scene> Sptr;
		
		// The camera for our scene
		Camera::Sptr               MainCamera;

		// Instead of a "base shader", we can specify a default material
		std::shared_ptr<Material>  DefaultMaterial;

		// Whether the application is in "play mode", lets us leverage editors!
		bool                       IsPlaying;

		bool IsDestroyed;

		Scene();
		~Scene();

		void SetPhysicsDebugDrawMode(BulletDebugMode mode);
		BulletDebugMode GetPhysicsDebugDrawMode() const;

		void SetSkyboxShader(const std::shared_ptr<ShaderProgram>& shader);
		std::shared_ptr<ShaderProgram> GetSkyboxShader() const;

		void SetSkyboxTexture(const std::shared_ptr<TextureCube>& texture);
		std::shared_ptr<TextureCube> GetSkyboxTexture() const;

		void SetSkyboxRotation(const glm::mat3& value);
		const glm::mat3& GetSkyboxRotation() const;

		void SetColorLUT(const Texture3D::Sptr& texture);
		const Texture3D::Sptr& GetColorLUT() const;

		/**
		 * Gets whether the scene has already called Awake()
		 */
		bool GetIsAwake() const { return _isAwake; }

		/// <summary>
		/// Creates a game object with the given name
		/// CreateGameObject is the only way to create game objects
		/// </summary>
		/// <param name="name">The name of the gameobject to create</param>
		/// <returns>A new gameobject with the given name</returns>
		GameObject::Sptr CreateGameObject(const std::string& name);

		/// <summary>
		/// Queues a game object for deletion at the call of the next Update function
		/// </summary>
		/// <param name="object">The gameobject to delete</param>
		void RemoveGameObject(const GameObject::Sptr& object);

		/// <summary>
		/// Searches all objects in the scene and returns the first
		/// one who's name matches the one given, or nullptr if no object
		/// is found
		/// </summary>
		/// <param name="name">The name of the object to find</param>
		GameObject::Sptr FindObjectByName(const std::string name) const;
		/// <summary>
		/// Searches all render objects in the scene and returns the first
		/// one who's guid matches the one given, or nullptr if no object
		/// is found
		/// </summary>
		/// <param name="id">The guid of the object to find</param>
		GameObject::Sptr FindObjectByGUID(Guid id) const;

		/// <summary>
		/// Sets the ambient light color for this scene
		/// </summary>
		/// <param name="value">The new value for the ambient light, should be in the 0-1 range</param>
		void SetAmbientLight(const glm::vec3& value);
		/// <summary>
		/// Gets the current ambient lighting factor for this scene
		/// </summary>
		const glm::vec3& GetAmbientLight() const;

		/// <summary>
		/// Gets the file path that this scene was saved to or loaded from
		/// </summary>
		const std::string& GetFilePath() const { return _filePath; }

		/// <summary>
		/// Calls awake on all objects in the scene,
		/// call this after loading or creating a new scene
		/// </summary>
		void Awake();

		/// <summary>
		/// Performs physics updates for all physics bodies in this scene,
		/// should be called after Update in the main loop
		/// 
		/// Only invokes events if IsPlaying is true
		/// </summary>
		/// <param name="dt">The time in seconds since the last frame</param>
		void DoPhysics(float dt);
		/// <summary>
		/// Renders debug information for the physics scene
		/// </summary>
		void DrawPhysicsDebug();

		/// <summary>
		/// Performs updates on all enabled components and gameobjects in the
		/// scene
		/// 
		/// Only invokes events if IsPlaying is true
		/// </summary>
		/// <param name="dt">The time in seconds since the last frame</param>
		void Update(float dt);

		/// <summary>
		/// Draws all GUI objects in the scene
		/// </summary>
		void RenderGUI();


		/// <summary>
		/// Draws ImGui stuff for all gameobjects in the scene
		/// </summary>
		void DrawAllGameObjectGUIs();

		void DrawSkybox();

		/// <summary>
		/// Gets the scene's Bullet physics world
		/// </summary>
		btDynamicsWorld* GetPhysicsWorld() const;

		/// <summary>
		/// Loads a scene from a JSON blob
		/// </summary>
		static Scene::Sptr FromJson(const nlohmann::json& data);
		/// <summary>
		/// Converts this object into it's JSON representation for storage
		/// </summary>
		nlohmann::json ToJson() const;

		ComponentManager& Components() { return _components; }
		const ComponentManager& Components() const { return _components; }

		/// <summary>
		/// Saves this scene to an output JSON file
		/// </summary>
		/// <param name="path">The path of the file to write to</param>
		void Save(const std::string& path);
		/// <summary>
		/// Loads a scene from an input JSON file
		/// </summary>
		/// <param name="path">The path of the file to read from</param>
		/// <returns>A new scene loaded from the file</returns>
		static Scene::Sptr Load(const std::string& path);


		int NumObjects() const;
		GameObject::Sptr GetObjectByIndex(int index) const;

	protected:
		friend class HierarchyWindow;
		friend class GameObject;

		// The component manager will store all components for objects in this scene
		ComponentManager _components;

		// Bullet physics stuff world
		btDynamicsWorld*          _physicsWorld;
		// Our bullet physics configuration
		btCollisionConfiguration* _collisionConfig; 
		// Handles dispatching collisions between objects
		btCollisionDispatcher*    _collisionDispatcher;
		// Provides rough broadphase (AABB) checks to improve performance
		btBroadphaseInterface*    _broadphaseInterface;
		// Resolves contraints (ex: hinge constraints, angle axis, etc...)
		btConstraintSolver*       _constraintSolver;
		// this is what allows us to get our pairs from the trigger volumes
		btGhostPairCallback*      _ghostCallback;

		BulletDebugDraw* _bulletDebugDraw;

		// The path that we've saved or loaded this scene from
		std::string             _filePath;

		// Our physics scene's global gravity, default matches earth's gravity (m/s^2)
		glm::vec3 _gravity;

		// Stores all the objects in our scene
		std::vector<GameObject::Sptr>  _objects;
		std::vector<std::weak_ptr<GameObject>>  _deletionQueue;

		// Info for rendering our skybox will be stored in the scene itself
		std::shared_ptr<ShaderProgram>       _skyboxShader;
		std::shared_ptr<MeshResource> _skyboxMesh;
		std::shared_ptr<TextureCube>  _skyboxTexture;
		glm::mat3                     _skyboxRotation;

		glm::vec3                     _ambientLight;

		// our LUT for color correction
		Texture3D::Sptr               _colorCorrection;

		bool                       _isAwake;

		/// <summary>
		/// Handles configuring our bullet physics stuff
		/// </summary>
		void _InitPhysics();
		/// <summary>
		/// Handles cleaning up bullet physics for this scene
		/// </summary>
		void _CleanupPhysics();

		void _FlushDeleteQueue();
	};
}