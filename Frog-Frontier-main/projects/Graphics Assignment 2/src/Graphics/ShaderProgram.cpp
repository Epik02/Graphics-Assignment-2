#include "ShaderProgram.h"
#include "Logging.h"
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"

ShaderProgram::ShaderProgram() : 
	IGraphicsResource(),
	IResource()
{
	_rendererId = glCreateProgram();
}

ShaderProgram::ShaderProgram(const std::unordered_map<ShaderPartType, std::string>& filePaths) :
	IGraphicsResource(),
	IResource()
{
	_rendererId = glCreateProgram();
	for (auto& [type, path] : filePaths) {
		LoadShaderPartFromFile(path.c_str(), type);
	}
	Link();
}

ShaderProgram::~ShaderProgram() {
	if (_rendererId != 0) {
		glDeleteProgram(_rendererId);
		_rendererId = 0;
	}
}

bool ShaderProgram::LoadShaderPart(const char* source, ShaderPartType type) {
	// Creates a new shader part (VS, FS, GS, etc...)
	GLuint handle = glCreateShader((GLenum)type);

	// Load the GLSL source and compile it
	glShaderSource(handle, 1, &source, nullptr);
	glCompileShader(handle);

	// Get the compilation status for the shader part
	GLint status = 0;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		// Get the size of the error log
		GLint logSize = 0;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);

		// Create a new character buffer for the log
		char* log = new char[logSize];

		// Get the log
		glGetShaderInfoLog(handle, logSize, &logSize, log);

		// Dump error log
		LOG_ERROR("Failed to compile shader part:\n{}", log);

		// Clean up our log memory
		delete[] log;

		// Delete the broken shader result
		glDeleteShader(handle);
		handle = 0;

		return false;
	}

	// If we're overwriting, warn and clean up the old program before we store
	if (_handles[type] != 0) {
		LOG_WARN("Another shader has been attached to this slot, overwriting");
		glDeleteShader(_handles[type]);
	}
	_handles[type] = handle;

	// Store info about where we got this data from
	_fileSourceMap[type].IsFilePath = false;
	_fileSourceMap[type].Source = source;

	return status != GL_FALSE;
}

bool ShaderProgram::LoadShaderPartFromFile(const char* path, ShaderPartType type) {
	// Make sure that the file exists before we try reading
	if (std::filesystem::exists(path)) {
		// Load the source from the file, using our helper that will
		// resolve #include directives
		std::string source = FileHelpers::ReadResolveIncludes(path);
		// Pass off to LoadShaderPart
		bool result =  LoadShaderPart(source.c_str(), type);
		_fileSourceMap[type].IsFilePath = true;
		_fileSourceMap[type].Source = path;
		if (result == false) {
			LOG_ERROR("Source File: {}", path);
		}
		glObjectLabel(GL_SHADER, _handles[type], -1, path);
		return result; 
	} else {
		LOG_WARN("Could not open file at \"{}\"", path);
		return false;
	}
}

bool ShaderProgram::Link() {

	LOG_TRACE("Starting shader link:");
	GLenum err = glGetError();
	
	// Attach all our shaders
	for (auto& [type, id] : _handles) {
		if (id != 0) {
			glAttachShader(_rendererId, id);
			LOG_TRACE("\t{} - {}", ~type, _fileSourceMap[type].IsFilePath ? _fileSourceMap[type].Source : "<from source>");
		}
	}

	// Perform linking
	glLinkProgram(_rendererId);
	err = glGetError();

	// Remove shader parts to save space (we can do this since we only needed the shader parts to compile an actual shader program)
	for (auto& [type, id] : _handles) { 
		if (id != 0) {
			glDetachShader(_rendererId, id);
			glDeleteShader(id);
		}
	}
	// Remove all the handles so we don't accidentally use them
	_handles.clear();

	err = glGetError();

	GLint status = 0;
	glGetProgramiv(_rendererId, GL_LINK_STATUS, &status);

	// If linking failed, figure out why
	if (status == GL_FALSE)
	{
		// Get the length of the log
		GLint length = 0;
		glGetProgramiv(_rendererId, GL_INFO_LOG_LENGTH, &length);

		if (length > 0) {
			// Read the log from openGL
			char* log = new char[length];
			glGetProgramInfoLog(_rendererId, length, &length, log);
			LOG_ERROR("Shader failed to link:\n{}", log);
			delete[] log; 
		} else {
			LOG_ERROR("Shader failed to link for an unknown reason!");
		}
	} else {
		LOG_TRACE("Linking complete, starting introspection");
	}

	// Perform our uniform introspection to see what uniforms are in the shader
	_Introspect();

	return status != GL_FALSE;
}

void ShaderProgram::Bind() {
	// Simply calls glUseProgram with our shader handle
	glUseProgram(_rendererId);
}

void ShaderProgram::Unbind() {
	// We unbind a shader program by using the default program (0)
	glUseProgram(0);
}

void ShaderProgram::SetUniformMatrix(int location, const glm::mat3* value, int count, bool transposed) {
	glProgramUniformMatrix3fv(_rendererId, location, count, transposed, glm::value_ptr(*value));
}
void ShaderProgram::SetUniformMatrix(int location, const glm::mat4* value, int count, bool transposed) {
	glProgramUniformMatrix4fv(_rendererId, location, count, transposed, glm::value_ptr(*value));
}

void ShaderProgram::SetUniform(int location, const float* value, int count) {
	glProgramUniform1fv(_rendererId, location, count, value);
}
void ShaderProgram::SetUniform(int location, const glm::vec2* value, int count) {
	glProgramUniform2fv(_rendererId, location, count, glm::value_ptr(*value));
}
void ShaderProgram::SetUniform(int location, const glm::vec3* value, int count) {
	glProgramUniform3fv(_rendererId, location, count, glm::value_ptr(*value));
}
void ShaderProgram::SetUniform(int location, const glm::vec4* value, int count) {
	glProgramUniform4fv(_rendererId, location, count, glm::value_ptr(*value));
}

void ShaderProgram::SetUniform(int location, const int* value, int count) {
	glProgramUniform1iv(_rendererId, location, count, value);
}
void ShaderProgram::SetUniform(int location, const glm::ivec2* value, int count) {
	glProgramUniform2iv(_rendererId, location, count, glm::value_ptr(*value));
}
void ShaderProgram::SetUniform(int location, const glm::ivec3* value, int count) {
	glProgramUniform3iv(_rendererId, location, count, glm::value_ptr(*value));
}
void ShaderProgram::SetUniform(int location, const glm::ivec4* value, int count) {
	glProgramUniform4iv(_rendererId, location, count, glm::value_ptr(*value));
}

void ShaderProgram::SetUniform(int location, const uint32_t* value, int count) {
	glProgramUniform1uiv(_rendererId, location, count, value);
}
void ShaderProgram::SetUniform(int location, const glm::uvec2* value, int count) {
	glProgramUniform2uiv(_rendererId, location, count, glm::value_ptr(*value));
}
void ShaderProgram::SetUniform(int location, const glm::uvec3* value, int count) {
	glProgramUniform3uiv(_rendererId, location, count, glm::value_ptr(*value));
}
void ShaderProgram::SetUniform(int location, const glm::uvec4* value, int count) {
	glProgramUniform4uiv(_rendererId, location, count, glm::value_ptr(*value));
}

void ShaderProgram::SetUniform(int location, const bool* value, int count) {
	LOG_ASSERT(count == 1, "SetUniform for bools only supports setting single values at a time!");
	glProgramUniform1i(location, *value, 1);
}
void ShaderProgram::SetUniform(int location, const glm::bvec2* value, int count) {
	LOG_ASSERT(count == 1, "SetUniform for bools only supports setting single values at a time!");
	glProgramUniform2i(location, value->x, value->y, 1);
}
void ShaderProgram::SetUniform(int location, const glm::bvec3* value, int count) {
	LOG_ASSERT(count == 1, "SetUniform for bools only supports setting single values at a time!");
	glProgramUniform3i(location, value->x, value->y, value->z, 1);
}
void ShaderProgram::SetUniform(int location, const glm::bvec4* value, int count) {
	LOG_ASSERT(count == 1, "SetUniform for bools only supports setting single values at a time!");
	glProgramUniform4i(location, value->x, value->y, value->z, value->w, 1);
}

void ShaderProgram::SetUniform(int location, ShaderDataType type, void* data, int count /*= 1*/, bool transposed  /* =false*/) {
	switch (type)
	{
		case ShaderDataType::Bool:    glProgramUniform1i(_rendererId, location, *static_cast<const bool*>(data)); break;
		case ShaderDataType::Bool2:   glProgramUniform2i(_rendererId, location, *static_cast<const bool*>(data), *(static_cast<const bool*>(data) + 1)); break;
		case ShaderDataType::Bool3:   glProgramUniform3i(_rendererId, location, *static_cast<const bool*>(data), *(static_cast<const bool*>(data) + 1), *(static_cast<const bool*>(data) + 2)); break;
		case ShaderDataType::Bool4:   glProgramUniform4i(_rendererId, location, *static_cast<const bool*>(data), *(static_cast<const bool*>(data) + 1), *(static_cast<const bool*>(data) + 2), *(static_cast<const bool*>(data) + 3)); break;
		case ShaderDataType::Float:   glProgramUniform1fv(_rendererId, location, count, static_cast<const float*>(data)); break;
		case ShaderDataType::Float2:  glProgramUniform2fv(_rendererId, location, count, static_cast<const float*>(data)); break;
		case ShaderDataType::Float3:  glProgramUniform3fv(_rendererId, location, count, static_cast<const float*>(data)); break;
		case ShaderDataType::Float4:  glProgramUniform4fv(_rendererId, location, count, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat2:    glProgramUniformMatrix2fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat3:    glProgramUniformMatrix3fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat4:    glProgramUniformMatrix4fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat2x3:  glProgramUniformMatrix2x3fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat2x4:  glProgramUniformMatrix2x4fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat3x2:  glProgramUniformMatrix3x2fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat3x4:  glProgramUniformMatrix3x4fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat4x2:  glProgramUniformMatrix4x2fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Mat4x3:  glProgramUniformMatrix4x3fv(_rendererId, location, count, transposed, static_cast<const float*>(data)); break;
		case ShaderDataType::Int:     glProgramUniform1iv(_rendererId, location, count, static_cast<const int*>(data)); break;
		case ShaderDataType::Int2:    glProgramUniform2iv(_rendererId, location, count, static_cast<const int*>(data)); break;
		case ShaderDataType::Int3:    glProgramUniform3iv(_rendererId, location, count, static_cast<const int*>(data)); break;
		case ShaderDataType::Int4:    glProgramUniform4iv(_rendererId, location, count, static_cast<const int*>(data)); break;
		case ShaderDataType::Uint:    glProgramUniform1uiv(_rendererId, location, count, static_cast<const uint32_t*>(data));  break;
		case ShaderDataType::Uint2:   glProgramUniform2uiv(_rendererId, location, count, static_cast<const uint32_t*>(data)); break;
		case ShaderDataType::Uint3:   glProgramUniform3uiv(_rendererId, location, count, static_cast<const uint32_t*>(data)); break;
		case ShaderDataType::Uint4:   glProgramUniform4uiv(_rendererId, location, count, static_cast<const uint32_t*>(data)); break;
		case ShaderDataType::Double:  glProgramUniform1dv(_rendererId, location, count, static_cast<const double*>(data)); break;
		case ShaderDataType::Double2: glProgramUniform2dv(_rendererId, location, count, static_cast<const double*>(data)); break;
		case ShaderDataType::Double3: glProgramUniform3dv(_rendererId, location, count, static_cast<const double*>(data)); break;
		case ShaderDataType::Double4: glProgramUniform4dv(_rendererId, location, count, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat2:   glProgramUniformMatrix2dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat3:   glProgramUniformMatrix3dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat4:   glProgramUniformMatrix4dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat2x3: glProgramUniformMatrix2x3dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat2x4: glProgramUniformMatrix2x4dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat3x2: glProgramUniformMatrix3x2dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat3x4: glProgramUniformMatrix3x4dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat4x2: glProgramUniformMatrix4x2dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
		case ShaderDataType::Dmat4x3: glProgramUniformMatrix4x3dv(_rendererId, location, count, transposed, static_cast<const double*>(data)); break;
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
			glProgramUniform1iv(_rendererId, location, count, static_cast<const int*>(data));
		case ShaderDataType::None: break;
		default:
		{
			static std::map<ShaderDataType, bool> loggedWarns;
			if (!loggedWarns[type]) {
				LOG_WARN("No support for uniforms of type \"{}\", skipping...", type);
				loggedWarns[type] = true;
			}
		}
	}
}

int ShaderProgram::__GetUniformLocation(const std::string& name) {
	// Since the default constructor for UniformInfo sets location to -1,
	// we can simply index the map and if it doesn't exist, the default
	// will be used
	return _uniforms[name].Location;
}

nlohmann::json ShaderProgram::ToJson() const {
	nlohmann::json result;
	result["name"] = _debugName;
	for (auto& [key, value] : _fileSourceMap) {
		result[~key][value.IsFilePath ? "path" : "source"] = value.Source;
	}
	return result;

}

ShaderProgram::Sptr ShaderProgram::FromJson(const nlohmann::json& data) {
	ShaderProgram::Sptr result = std::make_shared<ShaderProgram>();
	result->SetDebugName(JsonGet(data, "name", result->_debugName));
	for (auto& [key, blob] : data.items()) {
		// Get the shader part type from the key
		ShaderPartType type = ParseShaderPartType(key, ShaderPartType::Unknown);
		// As long as the type is valid
		if (type != ShaderPartType::Unknown) {
			// If it has a file, we load from file
			if (blob.contains("path")) {
				result->LoadShaderPartFromFile(blob["path"].get<std::string>().c_str(), type);
			}
			// Otherwise we see if there's a source and load that instead
			else if (blob.contains("source")) {
				result->LoadShaderPart(blob["source"].get<std::string>().c_str(), type);
			}
			// Otherwise do nothing
		}
	}
	result->Link();
	return result;
}

void ShaderProgram::_Introspect() {
	_IntrospectUniforms();
	_IntrospectUnifromBlocks();
}

void ShaderProgram::_IntrospectUniforms() {
	// Query the program for how many active uniforms we have
	int numInputs = 0;
	glGetProgramInterfaceiv(_rendererId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numInputs);

	// Iterate over all uniforms and extract some information
	for (int ix = 0; ix < numInputs; ix++) {
		// These are the parameters we want to collect
		static GLenum pNames[] ={
			GL_NAME_LENGTH,
			GL_TYPE,
			GL_ARRAY_SIZE,
			GL_LOCATION
		};
		// Allocate space for results
		int numProps = 0;
		int props[4];
		// Query the program about our uniform, passing data out to props
		glGetProgramResourceiv(_rendererId, GL_UNIFORM, ix, 4, pNames, 4, &numProps, props);

		// If location is -1, this is probably a uniform block element, ignore it
		if (props[3] == -1)
			continue;

		// Create a new Uniform Info
		UniformInfo e = UniformInfo();
		e.Name.resize(props[0] - 1);
		e.Binding = -1;

		// Query uniform name from the program
		int length = 0;
		glGetProgramResourceName(_rendererId, GL_UNIFORM, ix, props[0], &length, &e.Name[0]);

		// Store the other properties we got into the uniform info
		e.Type = FromGLShaderDataType(props[1]);
		e.Location = props[3];
		e.ArraySize = props[2];

		if (GetShaderDataTypeCode(e.Type) == ShaderDataTypecode::Texture) {			
			glGetUniformiv(_rendererId, e.Location, &e.Binding);
		}

		// If this is an array, we need to trim the [] off the name
		if (e.ArraySize > 1) {
			e.Name = e.Name.substr(0, e.Name.find('['));
		}
		// Trace is very low priority logs, we'll output our uniform info this way
		LOG_TRACE("\tDetected a new uniform: {} - {} -> {}[{}]", e.Location, e.Name, e.Type, e.ArraySize);

		// Store the uniform info
		_uniforms[e.Name] = e;
	}
}

void ShaderProgram::_IntrospectUnifromBlocks() {
	// Query program for the number of uniform blocks
	int numBlocks = 0;
	glGetProgramInterfaceiv(_rendererId, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);

	// Iterate over all blocks
	for (int ix = 0; ix < numBlocks; ix++) {
		// These are the properties that we want to extract
		static GLenum pNamesBlockProperties[] ={
			GL_NUM_ACTIVE_VARIABLES,
			GL_BUFFER_BINDING,
			GL_BUFFER_DATA_SIZE,
			GL_NAME_LENGTH
		};
		// Query the program for our information
		int results[4];
		glGetProgramResourceiv(_rendererId, GL_UNIFORM_BLOCK, ix, 4, pNamesBlockProperties, 4, NULL, results);

		// If this block has no active uniforms, skip it
		if (!results[0])
			continue;

		// We want to get all the handles to active uniforms in the block
		static GLenum pNamesActiveVars[] ={
			GL_ACTIVE_VARIABLES
		};
		// We can use a vector to store results, initializing it's size to the number of
		// active uniforms
		std::vector<int> activeVars(results[0]);
		glGetProgramResourceiv(_rendererId, GL_UNIFORM_BLOCK, ix, 1, pNamesActiveVars, results[0], NULL, activeVars.data());

		// Create the block and copy in the info from the shader
		UniformBlockInfo block = UniformBlockInfo();
		block.DefaultBinding = results[1];
		block.CurrentBinding = results[1];
		block.SizeInBytes = results[2];
		block.NumVariables = results[0];
		block.SubUniforms.reserve(results[0]);

		// Query block name from the program
		block.Name.resize(results[3] - 1);
		glGetProgramResourceName(_rendererId, GL_UNIFORM_BLOCK, ix, results[3], NULL, &block.Name[0]);

		// This is our block index for use when we bind uniform buffers to the block
		block.BlockIndex = glGetUniformBlockIndex(_rendererId, block.Name.c_str());

		LOG_TRACE("\tDetected a new uniform block \"{}\" with {} variables bound at {} ", block.Name, block.NumVariables, block.DefaultBinding);

		// Iterate over all the uniforms within the uniform block
		for (int v = 0; v < results[0]; v++) {
			// Parameters we wish to query from the uniform
			static GLenum pNames[] ={
				GL_NAME_LENGTH,
				GL_TYPE,
				GL_ARRAY_SIZE,
				GL_OFFSET
			};
			// Query data from the program
			int props[4];
			glGetProgramResourceiv(_rendererId, GL_UNIFORM, activeVars[v], 4, pNames, 4, NULL, props);

			// Store properties into the UniformInfo
			UniformInfo var = UniformInfo();
			var.Type = FromGLShaderDataType(props[1]);
			var.Location = props[3];
			var.ArraySize = props[2];

			// Get the uniform name
			var.Name.resize(props[0] - 1);
			glGetProgramResourceName(_rendererId, GL_UNIFORM, activeVars[v], props[0], NULL, &var.Name[0]);

			// If uuniform is array, remove the [0]
			if (var.ArraySize > 1) {
				var.Name = var.Name.substr(0, var.Name.find('['));
			}

			LOG_TRACE("\t\tDetected a new uniform: {}[{}] -> {} @ {}", var.Name, var.ArraySize, var.Type, var.Location);
			
			// Add uniform to the block
			block.SubUniforms.push_back(var);
		}

		_uniformBlocks[block.Name] = block;
	}
}

void ShaderProgram::BindUniformBlockToSlot(const std::string& name, int uboSlot)
{
	auto& it = _uniformBlocks.find(name);
	if (it != _uniformBlocks.end()) {
		UniformBlockInfo& block = it->second;
		glUniformBlockBinding(_rendererId, block.BlockIndex, uboSlot);
		block.CurrentBinding = uboSlot;
	}
}

bool ShaderProgram::FindUniform(const std::string& name, UniformInfo* out) {
	for (auto& [key, uniform] : _uniforms) {
		if (uniform.Name == name) {
			if (out != nullptr) {
				*out = uniform;
			}
			return true;
		}
	}
	return false;
}

GlResourceType ShaderProgram::GetResourceClass() const {
	return GlResourceType::ShaderProgram;
}

void ShaderProgram::RegisterVaryings(const char* const* names, int numVaryings, bool interleaved /*= true*/)
{
	glTransformFeedbackVaryings(_rendererId, numVaryings, names, interleaved ? GL_INTERLEAVED_ATTRIBS : GL_SEPARATE_ATTRIBS);
}
