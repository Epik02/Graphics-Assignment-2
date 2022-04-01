#pragma once
#include <memory>
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/ITexture.h"

namespace Gameplay {
	/// <summary>
	/// Helper structure for material parameters to our shader
	/// THIS IS VERY TEMPORARY
	/// </summary>
	class Material : public IResource {
	public:
		typedef std::shared_ptr<Material> Sptr;
		typedef std::weak_ptr<Material>   Wptr;

		/// <summary>
		/// We'll sometimes want to reserve some texture slots for shared textures, such
		/// as the environment map. We'll specify a number of reserved slots here
		/// </summary>
		static const int MAX_TEXTURE_SLOTS = 14;

		/// <summary>
		/// A human readable name for the material
		/// </summary>
		std::string     Name;

		/// <summary>
		/// Default constructor, to be used by Resource manager and smart pointers only
		/// </summary>
		Material();
		/// <summary>
		/// Creates a new material from a given shader
		/// </summary>
		/// <param name="shader">The shader for the material</param>
		Material(const ShaderProgram::Sptr& shader);

		/// <summary>
		/// Sets a material parameter with the given name and type
		/// </summary>
		/// <typeparam name="T">The type of parameter to set</typeparam>
		/// <param name="name">The name of the parameter, should match the uniform name</param>
		/// <param name="value">The value to set the parameter to</param>
		template <typename T>
		void Set(const std::string& name, const T& value) {
			ShaderDataType type = GetShaderDataType<T>();
			Set(name, type, &value, 1);
		}

		/// <summary>
		/// Sets a material parameter with the given name and type
		/// </summary>
		/// <param name="name">The name of the parameter, should match the uniform name</param>
		/// <param name="type">The type of uniform to set</param>
		/// <param name="value">A raw pointer to the underlying data to set the parameter to</param>
		/// <param name="arraySize">The array size in the event that the value is an array</param>
		void Set(const std::string& name, ShaderDataType type, const void* value, size_t arraySize = 1ul);

		/// <summary>
		/// Gets the shader that this material is using
		/// </summary>
		const ShaderProgram::Sptr& GetShader() const;

		/// <summary>
		/// Handles applying this material's state to the OpenGL pipeline
		/// Will bind the shader, update material uniforms, and bind textures
		/// </summary>
		virtual void Apply();

		/// <summary>
		/// Renders some UI controls for manipulating a material at runtime
		/// </summary>
		void RenderImGui();

		/// <summary>
		/// Creates a clone of this material, useful for cases where you have many similar 
		/// materials with slight variations
		/// </summary>
		Material::Sptr Clone() const;

		/// <summary>
		/// Loads a material from a JSON blob
		/// </summary>
		static Material::Sptr FromJson(const nlohmann::json& data);
		/// <summary>
		/// Converts this material into it's JSON representation for storage
		/// </summary>
		nlohmann::json ToJson() const;

	protected:
		/// <summary>
		/// Represents a single uniform that the material will control
		/// </summary>
		struct UniformData {
			// The name of the uniform in the shader
			std::string    Name;
			// Location of the uniform within the shader
			int            Location = -2;
			union {
				// A space to store non-array values, can store up to a dmat4
				uint8_t        Value[128];
				
				// A space to store arrays of values
				void*          ArrayBlock = nullptr;

				// For when the uniform is a texture
				ITexture::Sptr TextureAsset;
			};
			// The size of the array, in elements
			size_t         ArraySize;
			int            BindingSlot;

			// The type of uniform
			ShaderDataType Type = ShaderDataType::None;
			
			UniformData() :
				Name("<unknown>"),
				Location(-2),
				TextureAsset(nullptr),
				ArraySize(0),
				BindingSlot(-1),
				Type(ShaderDataType::None) 
			{ }
			UniformData(const UniformData& other);
			UniformData(UniformData&& other);
			UniformData& operator=(const UniformData& other);
			UniformData& operator=(UniformData&& other) noexcept;
			UniformData(const std::string& uniformName, const ShaderProgram::Sptr& shader);
			~UniformData();

			/// <summary>
			/// Renders GUI for this uniform
			/// </summary>
			bool RenderImGui();

			/// <summary>
			/// Converts this uniform into a JSON representation
			/// </summary>
			nlohmann::json ToJson() const;
			/// <summary>
			/// Parses a uniform information structure from a JSON blob
			/// </summary>
			/// <param name="blob">The JSON blob to parse</param>
			static UniformData FromJson(const nlohmann::json& blob, const std::string& name, const ShaderProgram::Sptr& shader);

			template <typename T>
			T& Get() {
				ShaderDataType tType = GetShaderDataType<T>();
				LOG_ASSERT(tType == Type, "Type mismatch");
				if constexpr (std::is_base_of<ITexture, T>::value) {
					return TextureAsset;
				} else {
					return *reinterpret_cast<T*>(Value);
				}
			}

			template <typename T>
			const T& Get() const {
				return const_cast<UniformData*>(this)->Get<T>();
			}

			/// <summary>
			/// Returns true if the uniform is a texture resource
			/// </summary>
			inline bool IsTextureResource() const {
				return GetShaderDataTypeCode(Type) == ShaderDataTypecode::Texture;
			}
		};
	
		/// <summary>
		/// The shader that the material is using
		/// </summary>
		ShaderProgram::Sptr    _shader;
		/// <summary>
		/// The uniforms that the material will be modifying
		/// </summary>
		std::unordered_map<std::string, UniformData> _uniforms;

		UniformData& _GetUniform(const std::string& name);
		void _PopulateUniforms();
	};
}