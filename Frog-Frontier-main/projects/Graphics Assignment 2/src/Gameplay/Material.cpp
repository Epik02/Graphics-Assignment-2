#include "Gameplay/Material.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/Textures/Texture2D.h"
#include "Logging.h"
#include "Utils/ImGuiHelper.h"
#include "Graphics/Textures/Texture1D.h"
#include "Graphics/Textures/Texture3D.h"

namespace Gameplay {
	Material::Material(const ShaderProgram::Sptr& shader) :
		IResource(),
		_shader(shader),
		_uniforms(std::unordered_map<std::string, UniformData>())
	{
		_PopulateUniforms();
	}

	Material::Material() :
		IResource(),
		_shader(nullptr),
		_uniforms(std::unordered_map<std::string, UniformData>())
	{ }

	void Material::Set(const std::string& name, ShaderDataType type, const void* value, size_t arraySize)
	{
		// Try and find the matching uniform
		UniformData& uniform = _GetUniform(name);

		// We have a uniform, let's see if we can update it
		if (uniform.Location != -2) {
			// If it's a texture, we update TextureAsset so it adds to the ref count
			if (GetShaderDataTypeCode(uniform.Type) == ShaderDataTypecode::Texture && type == ShaderDataType::None) {
				uniform.TextureAsset = *reinterpret_cast<const ITexture::Sptr*>(value);
			}
			// Check for type mismatch
			else if (uniform.Type != type && uniform.Type != ShaderDataType::None) {
				LOG_ERROR("Type mismatch for \"{}\", uniform is {}, passed {} in material \"{}\"", name, ~uniform.Type, ~type, Name);
			}
			// Types match, we're good to go
			else {
				// if it's an array, copy all the elements
				if (uniform.ArraySize > 1) {
					memcpy(uniform.ArrayBlock, value, ShaderDataTypeSize(type) * arraySize);
				} 
				// if it's just a value, copy the value
				else {
					memcpy(uniform.Value, value, ShaderDataTypeSize(type));
				}
			}
		}
		// We couldn't find that uniform, log a warning
		else {
			LOG_WARN("Failed to set parameter \"{}\" in material \"{}\", shader uniform not found", name, Name);
		}
	}

	const ShaderProgram::Sptr& Material::GetShader() const {
		return _shader;
	}

	void Material::Apply() {
		if (_shader != nullptr) {
			// Skip the reserved # of texture slots
			int textureSlot = 0;
			
			// Iterate over the uniforms map
			for (auto&[name, data] : _uniforms) {
				// The typecode is basically the underlying type of the uniform
				// ex: float, matrix, texture, etc...
				ShaderDataTypecode typeCode = GetShaderDataTypeCode(data.Type);

				// If the uniform is a texture, we try and bind it, then move to the next slot
				if (typeCode == ShaderDataTypecode::Texture) {
					if (textureSlot >= MAX_TEXTURE_SLOTS) {
						LOG_WARN("Ignoring material binding, exceeds allowed number of textures");
					}
					else {
						ITexture::Sptr texture = data.TextureAsset;
						if (texture != nullptr) {
							texture->Bind(textureSlot);
						}
						else {
							ITexture::Unbind(textureSlot);
						}
						// Send the slot to the shader
						_shader->SetUniform(data.Location, data.Type, &textureSlot);
						textureSlot++;
					}
				}
				// The uniform is a plain ol' value type, send it in
				else {
					_shader->SetUniform(data.Location, data.Type, data.ArraySize > 1 ? data.ArrayBlock : data.Value, data.ArraySize);
				}
			}
		}
	}

	void Material::RenderImGui() {
		ImGui::PushID(this);

		bool open = ImGui::CollapsingHeader(Name.c_str());

		ImGuiHelper::ResourceDragSource(this, Name);

		if (open) {
			ImGui::Text("Shader: %s", _shader != nullptr ? _shader->GetDebugName().c_str() : "null");
			// Draw all of our valid uniforms
			for (auto&[key, value] : _uniforms) {
				if (value.Location != -2 && value.Location != -1) {
					value.RenderImGui();
				}
			}

			// Slap a separator at the end 'cause why not
			ImGui::Separator();
		}

		ImGui::PopID();
	}

	Material::Sptr Material::Clone() const
	{
		// hehe, neat lil' hack
		return FromJson(ToJson());
	}

	Material::Sptr Material::FromJson(const nlohmann::json& data) {
		// Load in basic material info like shader and name
		Material::Sptr result = std::make_shared<Material>();
		result->OverrideGUID(Guid(data["guid"]));
		result->Name = data["name"].get<std::string>();
		result->_shader = ResourceManager::Get<ShaderProgram>(Guid(data["shader"]));
		result->_PopulateUniforms();

		// material specific parameters'
		if (data.contains("parameters") && data["parameters"].is_object()) {
			// Iterate over all objects
			for (auto& [key, value] : data["parameters"].items()) {
				// Try loading a uniform from the blob, if successful, store it
				Material::UniformData uniform = Material::UniformData::FromJson(value, key, result->_shader);
				if (uniform.Location != -2) {
					result->_uniforms[key] = uniform;
				}
			}
		}
		return result;
	}

	nlohmann::json Material::ToJson() const { 
		nlohmann::json result ={
			{ "guid", GetGUID().str() },
			{ "name", Name },
			{ "shader", _shader ? _shader->GetGUID().str() : "null" },
			{ "parameters", nlohmann::json() }
		};

		// Store all the uniforms
		for (auto& [key, value] : _uniforms) {
			if (value.Location != -1) {
				result["parameters"][key] = value.ToJson();
			}
		}

		return result;
	}

	Material::UniformData& Material::_GetUniform(const std::string& name)
	{
		UniformData& data = _uniforms[name];
		if (data.Location == -2) {
			ShaderProgram::UniformInfo uniform;
			if (_shader->FindUniform(name, &uniform)) {
				// Ignoring our reserved textures
				if (GetShaderDataTypeCode(uniform.Type) == ShaderDataTypecode::Texture && uniform.Binding >= MAX_TEXTURE_SLOTS) {
					data.Location = -1;
				}
				else {
					data = UniformData(name, _shader);
				}
			} else {
				data.Location = -1;
			}
		}
		return data;
	}

	void Material::_PopulateUniforms()
	{
		const auto& uniforms = _shader->GetUniforms();
		for (const auto& [key, value] : uniforms) {
			_uniforms[key] = _GetUniform(key);
		}
	}

	bool Material::UniformData::RenderImGui() {
		ImGui::PushID(Name.c_str());

		bool modified = false;

		// Will store names for fields
		static char buffer[256];

		// Get the typecode and number of elements within the type (ex: vec3 has 3 floats)
		ShaderDataTypecode typeCode = GetShaderDataTypeCode(Type);
		int numElements = ShaderDataTypeComponentCount(Type);

		// Determine if the backing field is Value or ArrayBlock
		uint8_t* dataStore = ArraySize > 1 ? (uint8_t*)ArrayBlock : Value;

		// We'll need the name regardless, create it here
		sprintf_s(buffer, "%s:", Name.c_str());

		// If this is an array, draw name and indent items
		if (ArraySize > 1) {
			ImGui::Text(buffer);
			ImGui::Indent();
		} 


		// Iterate over all elements in the array (or loop once if not an array)
		for (int ix = 0; ix < ArraySize; ix++) {
			// If it's an array element, the name is the index
			if (ArraySize > 1) {
				sprintf_s(buffer, "[%d]:", ix);
			}

			// For arrays determine our data offset
			uint8_t* elem = dataStore + (ShaderDataTypeSize(Type) * ix);

			// Push array index as another ID and start the input group
			ImGui::PushID(ix);
			ImGui::BeginGroup();

			// Render the label text, consume half of available space
			ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 2.0f);
			ImGui::Text(buffer);
			ImGui::PopItemWidth();

			// Next element should be on the same line
			ImGui::SameLine();

			// Calculate the size of each field
			float elemSize = ImGui::GetContentRegionAvailWidth();
			elemSize /= numElements;
			ImGui::PushItemWidth(elemSize);
			
			// Draw corresponding controls
			switch (typeCode)
			{
				case ShaderDataTypecode::Bool:
					for (int e = 0; e < numElements; e++) {
						ImGui::PushID(e);
						modified |= ImGui::Checkbox("", ((bool*)elem) + e);
						if (e < numElements - 1) { ImGui::SameLine(); }
						ImGui::PopID();
					}
					break;
				case ShaderDataTypecode::Float:
					for (int e = 0; e < numElements; e++) {
						ImGui::PushID(e);
						modified |= ImGui::DragFloat("", ((float*)elem) + e, 0.1f);
						if (e < numElements - 1) { ImGui::SameLine(); }
						ImGui::PopID();
					}
					break;
				case ShaderDataTypecode::Double:
					for (int e = 0; e < numElements; e++) {
						ImGui::PushID(e);
						modified |= ImGui::DragScalar("", ImGuiDataType_Double, ((double*)elem) + e, 0.1f);
						if (e < numElements - 1) { ImGui::SameLine(); }
						ImGui::PopID();
					}
					break;
				case ShaderDataTypecode::Int:
					for (int e = 0; e < numElements; e++) {
						ImGui::PushID(e);
						modified |= ImGui::DragScalar("", ImGuiDataType_S32, ((int*)elem) + e, 0.1f);
						if (e < numElements - 1) { ImGui::SameLine(); }
						ImGui::PopID();
					}
					break;
				case ShaderDataTypecode::Uint:
					for (int e = 0; e < numElements; e++) {
						ImGui::PushID(e);
						modified |= ImGui::DragScalar("", ImGuiDataType_U32, ((int*)elem) + e, 0.1f);
						if (e < numElements - 1) { ImGui::SameLine(); }
						ImGui::PopID();
					}
					break;
				case ShaderDataTypecode::Texture:
				{
					switch (Type)
					{
						break;
					case ShaderDataType::Tex2D:
					case ShaderDataType::Tex2D_Multisample:
					case ShaderDataType::Tex2D_Int:
					case ShaderDataType::Tex2D_Uint: 
					{
						Texture2D::Sptr tex = std::dynamic_pointer_cast<Texture2D>(TextureAsset);
						if (ImGuiHelper::DrawTextureDrop(tex, ImVec2(ImGui::GetTextLineHeight() * 2, ImGui::GetTextLineHeight() * 2))) {
							TextureAsset = tex;
						}
					}
						break;
						break;
					case ShaderDataType::Tex3D:
						break;
					case ShaderDataType::TexCube:
						break;
					case ShaderDataType::TexCubeShadow:
						break;
					case ShaderDataType::Tex1D_Int:
						break;
					case ShaderDataType::Tex1D_Int_Array:
						break;
						break;
					case ShaderDataType::Tex2D_Int_Rect:
						break;
					case ShaderDataType::Tex2D_Int_Array:
						break;
					case ShaderDataType::Tex2D_Int_Multisample:
						break;
					case ShaderDataType::Tex2D_Int_MultisampleArray:
						break;
					case ShaderDataType::Tex3D_Int:
						break;
					case ShaderDataType::TexCube_Int:
						break;
					case ShaderDataType::Tex1D_Uint:
						break;
					case ShaderDataType::Tex2D_Uint_Rect:
						break;
					case ShaderDataType::Tex1D_Uint_Array:
						break;
						break;
					case ShaderDataType::Tex2D_Uint_Array:
						break;
					case ShaderDataType::Tex2D_Uint_Multisample:
						break;
					case ShaderDataType::Tex2D_Uint_MultisampleArray:
						break;
					case ShaderDataType::Tex3D_Uint:
						break;
					case ShaderDataType::TexCube_Uint:
						break;
					case ShaderDataType::BufferTexture:
						break;
					case ShaderDataType::BufferTextureInt:
						break;
					case ShaderDataType::BufferTextureUint:
						break;
					default:
						break;
					}
				}
				default:
					break;
			}
			
			// Clear item width, end the input group, pop ID scope
			ImGui::PopItemWidth();
			ImGui::EndGroup();
			ImGui::PopID();
		}

		// If this was an array, pop the indent
		if (ArraySize > 1) {
			ImGui::Unindent();
		}
		ImGui::PopID();

		return modified;
	}

	////////////////////////////////////////////////////////////////
	// Below here be horrible boilerplate crap, enter at own risk //
	////////////////////////////////////////////////////////////////

	Material::UniformData& Material::UniformData::operator=(const Material::UniformData& other) {
		if (this != &other) {
			this->~UniformData();
			new(this) Material::UniformData(other);
		}
		return *this;
	}

	Material::UniformData& Material::UniformData::operator=(Material::UniformData&& other) noexcept {
		if (this != &other) {
			this->~UniformData();
			new(this) Material::UniformData(other);
		}
		return *this;
	}

	Material::UniformData::UniformData(const std::string& uniformName, const ShaderProgram::Sptr& shader) :
		TextureAsset(nullptr)
	{
		// We extract the uniform info from the shader to populate our info
		ShaderProgram::UniformInfo uniform;
		if (shader != nullptr && shader->FindUniform(uniformName, &uniform)) {
			Name = uniformName;
			Location = uniform.Location;
			Type = uniform.Type;
			ArraySize = uniform.ArraySize;
			BindingSlot = uniform.Binding;
			
			// Allocate memory for array if the uniform is an array
			if (ArraySize > 1) {
				ArrayBlock = malloc(ShaderDataTypeSize(Type) * ArraySize);
			}
		}
	}

	Material::UniformData::UniformData(const UniformData& other) :
		TextureAsset(nullptr) 
	{
		Name = other.Name;
		Location = other.Location;
		ArraySize = other.ArraySize;
		Type = other.Type;

		if (GetShaderDataTypeCode(Type) == ShaderDataTypecode::Texture) {
			TextureAsset = other.TextureAsset;
			LOG_ASSERT(ArraySize == 1, "Cannot currently handle arrays of samplers!");
		}
		if (ArraySize > 1) {
			ArrayBlock = malloc(ShaderDataTypeSize(Type) * ArraySize);
			memcpy(ArrayBlock, other.ArrayBlock, ShaderDataTypeSize(Type) * ArraySize);
		} else {
			memcpy(Value, other.Value, ShaderDataTypeSize(Type));
		}
	}

	Material::UniformData::UniformData(UniformData&& other) :
		TextureAsset(nullptr) 
	{
		Name      = other.Name;
		Location  = other.Location;
		ArraySize = other.ArraySize;
		Type      = other.Type;

		if (GetShaderDataTypeCode(Type) == ShaderDataTypecode::Texture) {
			TextureAsset = other.TextureAsset;
			LOG_ASSERT(ArraySize == 1, "Cannot currently handle arrays of samplers!");
		}
		if (ArraySize > 1) {
			ArrayBlock = other.ArrayBlock;
			other.ArrayBlock = nullptr;
			other.ArraySize  = 0;
		} else {
			memcpy(Value, other.Value, ShaderDataTypeSize(Type));
		}
	}

	Material::UniformData::~UniformData()
	{
		// Explicitly release the texture asset handle, since it's in a union
		if (GetShaderDataTypeCode(Type) == ShaderDataTypecode::Texture) {
			TextureAsset = nullptr;
		} 
		// If we allocated an array of memory, free it now
		else if (ArraySize > 1 && ArrayBlock != nullptr) {
			free(ArrayBlock);
		}
	}

	nlohmann::json Material::UniformData::ToJson() const {
		nlohmann::json result = nlohmann::json();
		result["type"] = ~Type;
		switch (Type) {
			case ShaderDataType::Float:
				result["value"] = Get<float>();
				break;
			case ShaderDataType::Float2:
				result["value"] = Get<glm::vec2>();
				break;
			case ShaderDataType::Float3:
				result["value"] = Get<glm::vec3>();
				break;
			case ShaderDataType::Float4:
				result["value"] = Get<glm::vec4>();
				break;
			case ShaderDataType::Mat2:
				result["value"] = (Get<glm::mat2>());
				break;
			case ShaderDataType::Mat3: 
				result["value"] = (Get<glm::mat3>());
				break;
			case ShaderDataType::Mat4:
				result["value"] = (Get<glm::mat4>());
				break;
			case ShaderDataType::Mat2x3:
				result["value"] = (Get<glm::mat2x3>());
				break;
			case ShaderDataType::Mat2x4:
				result["value"] = (Get<glm::mat2x4>());
				break;
			case ShaderDataType::Mat3x2:
				result["value"] = (Get<glm::mat3x2>());
				break;
			case ShaderDataType::Mat3x4:
				result["value"] = (Get<glm::mat3x4>());
				break;
			case ShaderDataType::Mat4x2:
				result["value"] = (Get<glm::mat4x2>());
				break;
			case ShaderDataType::Mat4x3:
				result["value"] = (Get<glm::mat4x3>());
				break;
			case ShaderDataType::Int:
				result["value"] = (Get<int>());
				break;
			case ShaderDataType::Int2:
				result["value"] = Get<glm::ivec2>();
				break;
			case ShaderDataType::Int3:
				result["value"] = Get<glm::ivec3>();
				break;
			case ShaderDataType::Int4:
				result["value"] = Get<glm::ivec4>();
				break;
			case ShaderDataType::Uint:
				result["value"] = (Get<unsigned int>());
				break;
			case ShaderDataType::Uint2:
				result["value"] = (Get<glm::uvec2>());
				break;
			case ShaderDataType::Uint3:
				result["value"] = (Get<glm::uvec3>());
				break;
			case ShaderDataType::Uint4:
				result["value"] = (Get<glm::uvec4>());
				break;
			case ShaderDataType::Uint64:
				result["value"] = (Get<uint64_t>());
				break;
			case ShaderDataType::Double:
				result["value"] = (Get<double>());
				break;
			case ShaderDataType::Double2:
				result["value"] = (Get<glm::dvec2>());
				break;
			case ShaderDataType::Double3:
				result["value"] = (Get<glm::dvec3>());
				break;
			case ShaderDataType::Double4:
				result["value"] = (Get<glm::dvec4>());
				break;
			case ShaderDataType::Dmat2:
				result["value"] = (Get<glm::dmat2>());
				break;
			case ShaderDataType::Dmat3:
				result["value"] = (Get<glm::dmat3>());
				break;
			case ShaderDataType::Dmat4:
				result["value"] = (Get<glm::dmat4>());
				break;
			case ShaderDataType::Dmat2x3:
				result["value"] = (Get<glm::dmat2x3>());
				break;
			case ShaderDataType::Dmat2x4:
				result["value"] = (Get<glm::dmat2x4>());
				break;
			case ShaderDataType::Dmat3x2:
				result["value"] = (Get<glm::dmat3x2>());
				break;
			case ShaderDataType::Dmat3x4:
				result["value"] = (Get<glm::dmat3x4>());
				break;
			case ShaderDataType::Dmat4x2:
				result["value"] = (Get<glm::dmat4x2>());
				break;
			case ShaderDataType::Dmat4x3:
				result["value"] = (Get<glm::dmat4x3>());
				break;
			case ShaderDataType::Bool:
				result["value"] = (Get<bool>());
				break;
			case ShaderDataType::Bool2:
				result["value"] = (Get<glm::bvec2>());
				break;
			case ShaderDataType::Bool3:
				result["value"] = (Get<glm::bvec3>());
				break;
			case ShaderDataType::Bool4:
				result["value"] = (Get<glm::bvec4>());
				break;
			case ShaderDataType::Tex1D:
			case ShaderDataType::Tex1D_Array:
			case ShaderDataType::Tex1D_Shadow:
			case ShaderDataType::Tex1D_ShadowArray:
			case ShaderDataType::Tex2D:
			case ShaderDataType::Tex2D_Rect:
			case ShaderDataType::Tex2D_Rect_Shadow:
			case ShaderDataType::Tex2D_Array:
			case ShaderDataType::Tex2D_Shadow:
			case ShaderDataType::Tex2D_ShadowArray:
			case ShaderDataType::Tex2D_Multisample:
			case ShaderDataType::Tex2D_MultisampleArray:
			case ShaderDataType::Tex3D:
			case ShaderDataType::TexCube:
			case ShaderDataType::TexCubeShadow:
			case ShaderDataType::Tex1D_Int:
			case ShaderDataType::Tex1D_Int_Array:
			case ShaderDataType::Tex2D_Int:
			case ShaderDataType::Tex2D_Int_Rect:
			case ShaderDataType::Tex2D_Int_Array:
			case ShaderDataType::Tex2D_Int_Multisample:
			case ShaderDataType::Tex2D_Int_MultisampleArray:
			case ShaderDataType::Tex3D_Int:
			case ShaderDataType::TexCube_Int:
			case ShaderDataType::Tex1D_Uint:
			case ShaderDataType::Tex2D_Uint_Rect:
			case ShaderDataType::Tex1D_Uint_Array:
			case ShaderDataType::Tex2D_Uint:
			case ShaderDataType::Tex2D_Uint_Array:
			case ShaderDataType::Tex2D_Uint_Multisample:
			case ShaderDataType::Tex2D_Uint_MultisampleArray:
			case ShaderDataType::Tex3D_Uint:
			case ShaderDataType::TexCube_Uint:
			case ShaderDataType::BufferTexture:
			case ShaderDataType::BufferTextureInt:
			case ShaderDataType::BufferTextureUint:
				result["value"] = TextureAsset ? TextureAsset->GetGUID().str() : "null";
				break;
			case ShaderDataType::None:
			default:
				LOG_WARN("Failed to serialize uniform \"{}\" with unknown type", Name);
				break;
		}
		return result;
	}

	Material::UniformData Material::UniformData::FromJson(const nlohmann::json& blob, const std::string& name, const ShaderProgram::Sptr& shader) {
		ShaderDataType type = ParseShaderDataType(JsonGet<std::string>(blob, "type"), ShaderDataType::None);
		if (type == ShaderDataType::None) {
			return Material::UniformData();
		}
		Material::UniformData result = Material::UniformData(name, shader);
		
		switch (type)
		{
			case ShaderDataType::Float:
				result.Get<float>() = blob["value"].get<float>();
				break;
			case ShaderDataType::Float2:
				result.Get<glm::vec2>() =blob["value"];
				break;
			case ShaderDataType::Float3:
				result.Get<glm::vec3>() = blob["value"];
				break;
			case ShaderDataType::Float4:
				result.Get<glm::vec4>() = blob["value"];
				break;
			case ShaderDataType::Mat2:
				result.Get<glm::mat2>() = blob["value"];
				break;
			case ShaderDataType::Mat3:
				result.Get<glm::mat3>() = blob["value"];
				break;
			case ShaderDataType::Mat4:
				result.Get<glm::mat4>() = (blob["value"]);
				break;
			case ShaderDataType::Mat2x3:
				result.Get<glm::mat2x3>() = (blob["value"]);
				break;
			case ShaderDataType::Mat2x4:
				result.Get<glm::mat2x4>() = (blob["value"]);
				break;
			case ShaderDataType::Mat3x2:
				result.Get<glm::mat3x2>() = (blob["value"]);
				break;
			case ShaderDataType::Mat3x4:
				result.Get<glm::mat3x4>() = (blob["value"]);
				break;
			case ShaderDataType::Mat4x2:
				result.Get<glm::mat4x2>() = (blob["value"]);
				break;
			case ShaderDataType::Mat4x3:
				result.Get<glm::mat4x3>() = (blob["value"]);
				break;
			case ShaderDataType::Int:
				result.Get<int>() = blob["value"].get<int>();
				break;
			case ShaderDataType::Int2:
				result.Get<glm::ivec2>() = (blob["value"]);
				break;
			case ShaderDataType::Int3:
				result.Get<glm::ivec3>() = (blob["value"]);
				break;
			case ShaderDataType::Int4:
				result.Get<glm::ivec4>() = (blob["value"]);
				break;
			case ShaderDataType::Uint:
				result.Get<uint32_t>() = blob["value"].get<uint32_t>();
				break;
			case ShaderDataType::Uint2:
				result.Get<glm::uvec2>() = (blob["value"]);
				break;
			case ShaderDataType::Uint3:
				result.Get<glm::uvec3>() = (blob["value"]);
				break;
			case ShaderDataType::Uint4:
				result.Get<glm::uvec4>() = (blob["value"]);
				break;
			case ShaderDataType::Uint64:
				result.Get<uint64_t>() = blob["value"].get<uint64_t>();
				break;
			case ShaderDataType::Double:
				result.Get<double>() = blob["value"].get<double>();
				break;
			case ShaderDataType::Double2:
				result.Get<glm::dvec2>() = (blob["value"]);
				break;
			case ShaderDataType::Double3:
				result.Get<glm::dvec3>() = (blob["value"]);
				break;
			case ShaderDataType::Double4:
				result.Get<glm::dvec4>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat2:
				result.Get<glm::dmat2>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat3:
				result.Get<glm::dmat3>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat4:
				result.Get<glm::dmat4>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat2x3:
				result.Get<glm::dmat2x3>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat2x4:
				result.Get<glm::dmat2x4>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat3x2:
				result.Get<glm::dmat3x2>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat3x4:
				result.Get<glm::dmat3x4>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat4x2:
				result.Get<glm::dmat4x2>() = (blob["value"]);
				break;
			case ShaderDataType::Dmat4x3:
				result.Get<glm::dmat4x3>() = (blob["value"]);
				break;
			case ShaderDataType::Bool:
				result.Get<bool>() = blob["value"].get<bool>();
				break;
			case ShaderDataType::Bool2:
				result.Get<glm::bvec2>() = (blob["value"]);
				break;
			case ShaderDataType::Bool3:
				result.Get<glm::bvec3>() = (blob["value"]);
				break;
			case ShaderDataType::Bool4:
				result.Get<glm::bvec4>() = (blob["value"]);
				break;
			case ShaderDataType::Tex2D:
			case ShaderDataType::Tex2D_Multisample:
			case ShaderDataType::Tex2D_Int:
			case ShaderDataType::Tex2D_Uint:
			case ShaderDataType::Tex2D_Uint_Multisample:
			case ShaderDataType::Tex2D_Int_Multisample:
				result.TextureAsset = ResourceManager::Get<Texture2D>(Guid(blob["value"].get<std::string>()));
				break;
			case ShaderDataType::TexCube:
			case ShaderDataType::TexCube_Uint:
			case ShaderDataType::TexCube_Int:
				result.TextureAsset = ResourceManager::Get<TextureCube>(Guid(blob["value"].get<std::string>()));
				break;
			case ShaderDataType::Tex1D:
			case ShaderDataType::Tex1D_Int:
			case ShaderDataType::Tex1D_Uint:
				result.TextureAsset = ResourceManager::Get<Texture1D>(Guid(blob["value"].get<std::string>()));
				break;
			case ShaderDataType::Tex3D:
			case ShaderDataType::Tex3D_Int:
			case ShaderDataType::Tex3D_Uint:
				result.TextureAsset = ResourceManager::Get<Texture3D>(Guid(blob["value"].get<std::string>()));
				break;
			case ShaderDataType::Tex1D_Array:
			case ShaderDataType::Tex1D_Shadow:
			case ShaderDataType::Tex1D_ShadowArray:
			case ShaderDataType::Tex2D_Rect:
			case ShaderDataType::Tex2D_Rect_Shadow:
			case ShaderDataType::Tex2D_Array:
			case ShaderDataType::Tex2D_Shadow:
			case ShaderDataType::Tex2D_ShadowArray:
			case ShaderDataType::Tex2D_MultisampleArray:
			case ShaderDataType::TexCubeShadow:
			case ShaderDataType::Tex1D_Int_Array:
			case ShaderDataType::Tex2D_Int_Rect:
			case ShaderDataType::Tex2D_Int_Array:
			case ShaderDataType::Tex2D_Int_MultisampleArray:
			case ShaderDataType::Tex2D_Uint_Rect:
			case ShaderDataType::Tex1D_Uint_Array:
			case ShaderDataType::Tex2D_Uint_Array:
			case ShaderDataType::Tex2D_Uint_MultisampleArray:
			case ShaderDataType::BufferTexture:
			case ShaderDataType::BufferTextureInt:
			case ShaderDataType::BufferTextureUint:
			case ShaderDataType::None:
				break;
				break;
		}
		
		return result;
	}
}