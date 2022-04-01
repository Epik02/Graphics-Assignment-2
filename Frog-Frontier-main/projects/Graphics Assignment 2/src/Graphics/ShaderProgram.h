#pragma once
#include <glad/glad.h>
#include <memory>
#include <string>               // for std::string
#include <unordered_map>        // for std::unordered_map
#include <GLM/glm.hpp>          // for our GLM types
#include <GLM/gtc/type_ptr.hpp> // for glm::value_ptr
#include <Logging.h>            // for the logging functions
#include <EnumToString.h>

#include "Utils/ResourceManager/IResource.h"
#include "Graphics/GlEnums.h"
#include "Graphics/IGraphicsResource.h"

/// <summary>
/// This class will wrap around an OpenGL shader program
/// </summary>
class ShaderProgram final : public IGraphicsResource, public IResource
{
public:
	DEFINE_RESOURCE(ShaderProgram);

	static inline Sptr Create() {
		return std::make_shared<ShaderProgram>();
	}

public:
	// Stores information about a uniform in the shader
	struct UniformInfo {
		ShaderDataType Type;
		int            ArraySize;
		int            Location;
		int            Binding;
		std::string    Name;

		UniformInfo() :
			Type(ShaderDataType::None),
			ArraySize(0),
			Location(-1),
			Binding(-1),
			Name("") {}
	};

	/// <summary>
	/// Stores information about uniform blocks in the shader
	/// Uniform blocks are structures of data which we can feed
	/// directly from our application, as well as letting us share
	/// buffers of uniforms between shader programs
	/// </summary>
	struct UniformBlockInfo {
		std::string Name;
		int         DefaultBinding;
		int         CurrentBinding;
		int         BlockIndex;
		int         SizeInBytes;
		int         NumVariables;

		std::vector<UniformInfo> SubUniforms;
	};
	
public:
	/// <summary>
	/// Creates a new empty shader object
	/// </summary>
	ShaderProgram();

	ShaderProgram(const std::unordered_map<ShaderPartType, std::string>& filePaths);

	// Note, we don't need to make this virtual since this class is marked final (basically it can't be used as a base class)
	~ShaderProgram();

	/// <summary>
	/// Loads a single shader stage into this shader object (ex: Vertex Shader or Fragment Shader)
	/// </summary>
	/// <param name="source">The source code of the shader to load</param>
	/// <param name="type">The stage to load (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)</param>
	/// <returns>True if the shader is loaded, false if there was an issue</returns>
	bool LoadShaderPart(const char* source, ShaderPartType type);
	/// <summary>
	/// Loads a single shader stage into this shader object (ex: Vertex Shader or Fragment Shader) from an external file (in res)
	/// </summary>
	/// <param name="path">The relative path to the file containing the source</param>
	/// <param name="type">The stage to load (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)</param>
	/// <returns>True if the shader is loaded, false if there was an issue</returns>
	bool LoadShaderPartFromFile(const char* path, ShaderPartType type);

	/// <summary>
	/// Registers a list of varying outputs to capture for transform feedback, must be called before Link
	/// </summary>
	/// <param name="names">An array of names to register as varying attributes for capture</param>
	/// <param name="numVaryings">The number of names in the names array</param>
	/// <param name="interleaved">True if the attributes should be interleaved into a single buffer</param>
	void RegisterVaryings(const char* const* names, int numVaryings, bool interleaved = true);

	/// <summary>
	/// Links the vertex and fragment shader, and allows this shader program to be used
	/// </summary>
	/// <returns>True if the linking was successful, false if otherwise</returns>
	bool Link();

	/// <summary>
	/// Binds this shader for use
	/// </summary>
	void Bind();
	/// <summary>
	/// Unbinds all shader programs
	/// </summary>
	static void Unbind();

	const std::unordered_map<std::string, UniformInfo>& GetUniforms() const { return _uniforms; }

	// Inherited from IGraphicsResource

	virtual GlResourceType GetResourceClass() const override;

	// Inherited from IResource

	virtual nlohmann::json ToJson() const override;
	static ShaderProgram::Sptr FromJson(const nlohmann::json& data);

public:
	bool FindUniform(const std::string& name, UniformInfo* out);

	void SetUniformMatrix(int location, const glm::mat3* value, int count = 1, bool transposed = false);
	void SetUniformMatrix(int location, const glm::mat4* value, int count = 1, bool transposed = false);
	void SetUniform(int location, const float* value, int count = 1);
	void SetUniform(int location, const glm::vec2* value, int count = 1);
	void SetUniform(int location, const glm::vec3* value, int count = 1);
	void SetUniform(int location, const glm::vec4* value, int count = 1);
	void SetUniform(int location, const int* value, int count = 1);
	void SetUniform(int location, const glm::ivec2* value, int count = 1);
	void SetUniform(int location, const glm::ivec3* value, int count = 1);
	void SetUniform(int location, const glm::ivec4* value, int count = 1);
	void SetUniform(int location, const uint32_t* value, int count = 1);
	void SetUniform(int location, const glm::uvec2* value, int count = 1);
	void SetUniform(int location, const glm::uvec3* value, int count = 1);
	void SetUniform(int location, const glm::uvec4* value, int count = 1);
	void SetUniform(int location, const bool* value, int count = 1);
	void SetUniform(int location, const glm::bvec2* value, int count = 1); 
	void SetUniform(int location, const glm::bvec3* value, int count = 1);
	void SetUniform(int location, const glm::bvec4* value, int count = 1);

	/// <summary>
	/// Sets a uniform based on a shader data type, can be used by our materials class
	/// </summary>
	/// <param name="location">The uniform location</param>
	/// <param name="type">The type of the uniform</param>
	/// <param name="data">A pointer to the data to upload</param>
	/// <param name="count">The size of the uniform array, in elements</param>
	/// <param name="transposed"True if matrices should be transposed</param>
	void SetUniform(int location, ShaderDataType type, void* data, int count = 1, bool transposed = false);

	template <typename T>
	void SetUniform(const std::string& name, const T& value) {
		int location = __GetUniformLocation(name);
		if (location != -1) {
			SetUniform(location, &value, 1);
		} else {
			LOG_WARN("Ignoring uniform \"{}\"", name);
		}
	}
	template <typename T>
	void SetUniform(const std::string& name, const T* values, int count = 1) {
		int location = __GetUniformLocation(name);
		if (location != -1) {
			SetUniform(location, values, count);
		} else {
			LOG_WARN("Ignoring uniform \"{}\"", name);
		}
	}
	template <typename T>
	void SetUniformMatrix(const std::string& name, const T& value, bool transposed = false) {
		int location = __GetUniformLocation(name);
		if (location != -1) {
			SetUniformMatrix(location, &value, 1, transposed);
		} else {
			LOG_WARN("Ignoring uniform \"{}\"", name);
		}
	}
	
	void BindUniformBlockToSlot(const std::string& name, int uboSlot);

protected:
	// Stores all the handles to our shaders until we
	// are ready to compile them into a program
	std::unordered_map<ShaderPartType, int> _handles;
	
	// Map access to look up uniform locations and blocks
	std::unordered_map<std::string, UniformInfo> _uniforms;
	std::unordered_map<std::string, UniformBlockInfo> _uniformBlocks;

	// Stores information about the source of our shader parts
	// EX: if a VS shader is loaded from a file, will contain
	// the file path, and IsFilePath=true
	struct ShaderSource {
		std::string Source;
		bool        IsFilePath;
	};
	std::unordered_map<ShaderPartType, ShaderSource> _fileSourceMap;

	/// <summary>
	/// Performs program introspection, where we examine the uniforms that
	/// the program contains
	/// </summary>
	void _Introspect();
	/// <summary>
	/// Introspects uniforms which are not part of uniform blocks
	/// </summary>
	void _IntrospectUniforms();
	/// <summary>
	/// Introspects uniform blocks, which are structures that can be
	/// fed data from a uniform buffer
	/// </summary>
	void _IntrospectUnifromBlocks();

	int __GetUniformLocation(const std::string& name);
};