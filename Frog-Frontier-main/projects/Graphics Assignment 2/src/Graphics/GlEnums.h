/*
* This file contains general purpose OpenGL enums that may be used in various places throughout our game engine
* The major feature is ShaderDataType, which allows us to gather information about GL Uniforms and their data
* structure
*/
#pragma once
#include <EnumToString.h>
#include <glad/glad.h>
#include <Logging.h>
#include <glm/glm.hpp>

// We can use an enum to make our code more readable and restrict
// values to only ones we want to accept
ENUM(ShaderPartType, GLint,
	 Vertex       = GL_VERTEX_SHADER,
	 Fragment     = GL_FRAGMENT_SHADER,
	 TessControl  = GL_TESS_CONTROL_SHADER,
	 TessEval     = GL_TESS_EVALUATION_SHADER,
	 Geometry     = GL_GEOMETRY_SHADER,
	 Unknown      = GL_NONE // Usually good practice to have an "unknown" or "none" state for enums
)

/// <summary>
/// The types of texture we will support in our framework
/// </summary>
/// <see>https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCreateTextures.xhtml</see>
ENUM(TextureType, GLenum,
	_1D            = GL_TEXTURE_1D,
	_2D            = GL_TEXTURE_2D,
	_3D            = GL_TEXTURE_3D,
	Cubemap        = GL_TEXTURE_CUBE_MAP,
	_2DMultisample = GL_TEXTURE_2D_MULTISAMPLE
)

// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
// These are some of our more common available internal formats
ENUM(InternalFormat, GLint,
	Unknown      = GL_NONE,
	Depth        = GL_DEPTH_COMPONENT,
	DepthStencil = GL_DEPTH_STENCIL,
	R8           = GL_R8,
	R16          = GL_R16,
	RG8          = GL_RG8,
	RGB8         = GL_RGB8,
	SRGB         = GL_SRGB8,
	RGB10        = GL_RGB10,
	RGB16        = GL_RGB16,
	RGB32F       = GL_RGB32F,
	RGBA8        = GL_RGBA8,
	SRGBA        = GL_SRGB8_ALPHA8,
	RGBA16       = GL_RGBA16,
	RGB32AF      = GL_RGBA32F
	// Note: There are sized internal formats but there is a LOT of them
)

// The layout of the input pixel data
ENUM(PixelFormat, GLint,
    Unknown      = GL_NONE,
	Red          = GL_RED,
	RG           = GL_RG,
	RGB          = GL_RGB,
	SRGB         = GL_SRGB,
	BGR          = GL_BGR,
	RGBA         = GL_RGBA,
	BGRA         = GL_BGRA,
	Depth        = GL_DEPTH_COMPONENT,
	DepthStencil = GL_DEPTH_STENCIL
)

// The type for each component of the pixel data
ENUM(PixelType, GLint,
	Unknown = GL_NONE,
	UByte   = GL_UNSIGNED_BYTE,
	Byte    = GL_BYTE,
	UShort  = GL_UNSIGNED_SHORT,
	Short   = GL_SHORT,
	UInt    = GL_UNSIGNED_INT,
	Int     = GL_INT,
	Float   = GL_FLOAT
)

// These are our options for GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_R
ENUM(WrapMode, GLint,
	Unknown           = GL_NONE,
	ClampToEdge       = GL_CLAMP_TO_EDGE,
	ClampToBorder     = GL_CLAMP_TO_BORDER,
	MirroredRepeat    = GL_MIRRORED_REPEAT,
	Repeat            = GL_REPEAT, // Default
	MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
)

// These are our available options for the GL_TEXTURE_MIN_FILTER setting
ENUM(MinFilter, GLint,
    Unknown           = GL_NONE,
	Nearest           = GL_NEAREST,
	Linear            = GL_LINEAR,
	NearestMipNearest = GL_NEAREST_MIPMAP_NEAREST,
	LinearMipNearest  = GL_LINEAR_MIPMAP_NEAREST,
	NearestMipLinear  = GL_NEAREST_MIPMAP_LINEAR, // This is the default setting
	LinearMipLinear   = GL_LINEAR_MIPMAP_LINEAR
)

// These are our available options for the GL_TEXTURE_MAG_FILTER setting
ENUM(MagFilter, GLint,
	Unknown           = GL_NONE,
	Nearest           = GL_NEAREST,
	Linear            = GL_LINEAR  // This is the default setting
)

/*
 * Gets the size of a single component in the given format, in bytes.
 */
constexpr size_t GetTexelComponentSize(PixelType type) {
	switch (type) {
	case PixelType::UByte:
	case PixelType::Byte:
		return 1;
	case PixelType::UShort:
	case PixelType::Short:
		return 2;
	case PixelType::Int:
	case PixelType::UInt:
	case PixelType::Float:
		return 4;
	default:
		LOG_ASSERT(false, "Unknown type: {}", type);
		return 0;
	}
}

constexpr InternalFormat GetInternalFormatForChannels8(int numChannels) {
	switch (numChannels) {
		case 1:
			return InternalFormat::R8;
		case 2:
			return InternalFormat::RG8;
		case 3:
			return InternalFormat::RGB8;
		case 4:
			return InternalFormat::RGBA8;
		default:
			LOG_WARN(false, "Unsupported texture format with {0} channels", numChannels);
			return InternalFormat::Unknown;
	}
}
constexpr PixelFormat GetPixelFormatForChannels(int numChannels) {
	switch (numChannels) {
		case 1:
			return PixelFormat::Red;
		case 2:
			return PixelFormat::RG;
		case 3:
			return PixelFormat::RGB;
		case 4:
			return PixelFormat::RGBA;
		default:
			LOG_WARN(false, "Unsupported texture format with {0} channels", numChannels);
			return PixelFormat::Unknown;
	}
}

/*
 * Gets the number of components in a given pixel format
 */
constexpr GLint GetTexelComponentCount(PixelFormat format) {
	switch (format) {
		case PixelFormat::Depth:
		case PixelFormat::DepthStencil:
		case PixelFormat::Red:
			return 1;
		case PixelFormat::RG:
			return 2;
		case PixelFormat::RGB:
		case PixelFormat::BGR:
			return 3;
		case PixelFormat::RGBA:
		case PixelFormat::BGRA:
			return 4;
		default:
			LOG_ASSERT(false, "Unknown format: {}", format);
			return 0;
	}
}

/*
 * Gets the number of bytes needed to represent a single texel of the given format and type
 * @param format The format of the texel
 * @param type The data type of the texel
 * @returns The size of a single texel of the given format and type, in bytes
 */
constexpr size_t GetTexelSize(PixelFormat format, PixelType type) {
	return GetTexelComponentSize(type) * GetTexelComponentCount(format);
}


/*
	* Represents the type of data used in a shader in a more useful format for us
	* than what OpenGL provides to us
	*
	* The bitwise makeup of these values is shown below:
	*
	* HIGH              12            6       3      LOW 
	* ┌───┬─────────────┬─────────────┬───────┬───────┐
	* │ 0 │ 0 0 0 0 0 0 │ 0 0 0 0 0 0 │ 0 0 0 │ 0 0 0 │
	* ├─┬─┼─────────────┼─────────────┼───────┼───────┤
	* │ │ │  Underlying │   RESERVED  │Column │  Row  │ 
	* │ │ │  Data Type  │ FOR TEXTURES│Length │Length │
 	* │ │ └─────────────┴─────────────┴───────┴───────┤
	* │ └ Set to 1 for texture types                  │
	* └───────────────────────────────────────────────┘
	*/
#pragma region ShaderDataType
ENUM(ShaderDataType, uint32_t,
		None = 0,

		Float   = 0b00000001'000000'000'001,
		Float2  = 0b00000001'000000'000'010,
		Float3  = 0b00000001'000000'000'011,
		Float4  = 0b00000001'000000'000'100,
		Mat2    = 0b00000010'000000'010'010,
		Mat3    = 0b00000010'000000'011'011,
		Mat4    = 0b00000010'000000'100'100,
		Mat2x3  = 0b00000010'000000'010'011,
		Mat2x4  = 0b00000010'000000'010'100,
		Mat3x2  = 0b00000010'000000'011'010,
		Mat3x4  = 0b00000010'000000'011'100,
		Mat4x2  = 0b00000010'000000'100'010,
		Mat4x3  = 0b00000010'000000'100'011,

		Int     = 0b00000100'000000'000'001,
		Int2    = 0b00000100'000000'000'010,
		Int3    = 0b00000100'000000'000'011,
		Int4    = 0b00000100'000000'000'100,

		Uint    = 0b00001000'000000'000'001,
		Uint2   = 0b00001000'000000'000'010,
		Uint3   = 0b00001000'000000'000'011,
		Uint4   = 0b00001000'000000'000'100,

		Uint64  = 0b00001000'000000'001'001,

		Double  = 0b00010000'000000'000'001,
		Double2 = 0b00010000'000000'000'010,
		Double3 = 0b00010000'000000'000'011,
		Double4 = 0b00010000'000000'000'100,

		// Note: Double precision matrices do require some newer RenderAPI's (mainly OpenGL4 or DX11 level hardware)

		Dmat2   = 0b00100000'00000'010'0010,
		Dmat3   = 0b00100000'00000'011'0011,
		Dmat4   = 0b00100000'00000'100'0100,
		Dmat2x3 = 0b00100000'00000'010'0011,
		Dmat2x4 = 0b00100000'00000'010'0100,
		Dmat3x2 = 0b00100000'00000'011'0010,
		Dmat3x4 = 0b00100000'00000'011'0100,
		Dmat4x2 = 0b00100000'00000'100'0010,
		Dmat4x3 = 0b00100000'00000'100'0011,

		Bool    = 0b01000000'00000'000'0001,
		Bool2   = 0b01000000'00000'000'0010,
		Bool3   = 0b01000000'00000'000'0011,
		Bool4   = 0b01000000'00000'000'0100,

		// Texture resources (not to be used in vertex elements)

		// Usage of bit fields are a bit different here
		// 	HIGH              12       8   7   6   5   4   3      LOW 
		// ┌─────────────────┬─────────┬───┬───┬───┬───┬───┬───────┐
		// | 1 0 0 0 0 0 0 0 │ 0 0 0 0 │ 0 │ 0 │ 0 │ 0 │ 0 │ 0 0 0 |
		// ├─────────────────┼─────────┼───┼───┼── ┼───┼───┼───────┤
		// │ Sampler         │ \    /  │ B │ M │ S │ A │ R │ \Size/│      
		// │                 │  Data   │ U │ U │ H │ R │ E │ or 100│                   
		// │                 │  TYPE   │ F │ L │ A │ R │ C │ for   │                    
		// │                 │ 0-DFLT  │ F │ T │ D │ A │ T │ cube  │                      
		// │                 │ 1-INT   │ E │ I │ O │ Y │   │       │
		// │                 │ 2-UINT  │ R │ S │ W │   │   │       │
		// │                 │         │   │ A │   │   │   │       │
		// │                 │         │   │ M │   │   │   │       │
		// │                 │         │   │ P │   │   │   │       │
		// │                 │         │   │ L │   │   │   │       │
		// │                 │         │   │ E │   │   │   │       │
		// └─────────────────┴─────────┴───┴───┴───┴───┴───┴───────┘                            

		// Rect texture means no mipmapping, texel fetch is non-normalized
		Tex1D                       = 0b10000000'0000'00000'001,
		Tex1D_Array                 = 0b10000000'0000'00010'001,
		Tex1D_Shadow                = 0b10000000'0000'00100'001,
		Tex1D_ShadowArray           = 0b10000000'0000'00110'001,
		Tex2D                       = 0b10000000'0000'00000'010,
		Tex2D_Rect                  = 0b10000000'0000'00001'010,
		Tex2D_Rect_Shadow           = 0b10000000'0000'00101'010,
		Tex2D_Array                 = 0b10000000'0000'00010'010,
		Tex2D_Shadow                = 0b10000000'0000'00100'010,
		Tex2D_ShadowArray           = 0b10000000'0000'00110'010,
		Tex2D_Multisample           = 0b10000000'0000'01000'010,
		Tex2D_MultisampleArray      = 0b10000000'0000'01010'010,

		Tex3D                       = 0b10000000'0000'00000'011,

		TexCube                     = 0b10000000'0000'00000'100,
		TexCubeShadow               = 0b10000000'0000'00100'100,

		Tex1D_Int                   = 0b10000000'0001'00000'001,
		Tex1D_Int_Array             = 0b10000000'0001'00010'001,
		Tex2D_Int                   = 0b10000000'0001'00000'010,
		Tex2D_Int_Rect              = 0b10000000'0001'00001'010,
		Tex2D_Int_Array             = 0b10000000'0001'00010'010,
		Tex2D_Int_Multisample       = 0b10000000'0001'01000'010,
		Tex2D_Int_MultisampleArray  = 0b10000000'0001'01010'010,
		Tex3D_Int                   = 0b10000000'0001'00000'011,
		TexCube_Int                 = 0b10000000'0001'00000'100,

		Tex1D_Uint                  = 0b10000000'0010'00000'001,
		Tex2D_Uint_Rect             = 0b10000000'0010'00001'010,
		Tex1D_Uint_Array            = 0b10000000'0010'00010'001,
		Tex2D_Uint                  = 0b10000000'0010'00000'010,
		Tex2D_Uint_Array            = 0b10000000'0010'00010'010,
		Tex2D_Uint_Multisample      = 0b10000000'0010'01000'010,
		Tex2D_Uint_MultisampleArray = 0b10000000'0010'01010'010,
		Tex3D_Uint                  = 0b10000000'0010'00000'011,
		TexCube_Uint                = 0b10000000'0010'00000'100,

		// Buffer textures are for passing unfiltered data, can only be accessed via texelFetch
		BufferTexture               = 0b10000000'0000'10000'000,
		BufferTextureInt            = 0b10000000'0001'10000'000,
		BufferTextureUint           = 0b10000000'0010'10000'000
);
#pragma endregion

// These masks let us extract info from our ShaderDataType structures

const uint32_t ShaderDataType_TypeMask  = 0b11111111'000000'000'000;
const uint32_t ShaderDataType_Size1Mask = 0b00000000'000000'000'111;
const uint32_t ShaderDataType_Size2Mask = 0b00000000'000000'111'000;

/*
* This section let's us map built in and GLM types to their corresponding ShaderDataType
* 
* for intance, GetShaderDataType<float>() returns ShaderDataType::Float
*/
#pragma region Type to ShaderData Mapping
template <typename T>
constexpr ShaderDataType GetShaderDataType() {
	return ShaderDataType::None;
}

template <> constexpr inline ShaderDataType GetShaderDataType<float>() {
	return ShaderDataType::Float;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::vec2>() {
	return ShaderDataType::Float2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::vec3>() {
	return ShaderDataType::Float3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::vec4>() {
	return ShaderDataType::Float4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat2>() {
	return ShaderDataType::Mat2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat2x3>() {
	return ShaderDataType::Mat2x3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat2x4>() {
	return ShaderDataType::Mat2x4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat3>() {
	return ShaderDataType::Mat3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat3x2>() {
	return ShaderDataType::Mat3x2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat3x4>() {
	return ShaderDataType::Mat3x4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat4>() {
	return ShaderDataType::Mat4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat4x2>() {
	return ShaderDataType::Mat4x2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::mat4x3>() {
	return ShaderDataType::Mat4x3;
}

template <> constexpr inline ShaderDataType GetShaderDataType<double>() {
	return ShaderDataType::Double;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dvec2>() {
	return ShaderDataType::Double2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dvec3>() {
	return ShaderDataType::Double3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dvec4>() {
	return ShaderDataType::Double4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat2>() {
	return ShaderDataType::Dmat2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat2x3>() {
	return ShaderDataType::Dmat2x3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat2x4>() {
	return ShaderDataType::Dmat2x4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat3>() {
	return ShaderDataType::Dmat3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat3x2>() {
	return ShaderDataType::Dmat3x2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat3x4>() {
	return ShaderDataType::Dmat3x4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat4>() {
	return ShaderDataType::Dmat4;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat4x2>() {
	return ShaderDataType::Dmat4x2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::dmat4x3>() {
	return ShaderDataType::Dmat4x3;
}

template <> constexpr inline ShaderDataType GetShaderDataType<int>() {
	return ShaderDataType::Int;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::ivec2>() {
	return ShaderDataType::Int2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::ivec3>() {
	return ShaderDataType::Int3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::ivec4>() {
	return ShaderDataType::Int4;
}

template <> constexpr inline ShaderDataType GetShaderDataType<unsigned int>() {
	return ShaderDataType::Uint;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::uvec2>() {
	return ShaderDataType::Uint2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::uvec3>() {
	return ShaderDataType::Uint3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::uvec4>() {
	return ShaderDataType::Uint4;
}

template <> constexpr inline ShaderDataType GetShaderDataType<bool>() {
	return ShaderDataType::Bool;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::bvec2>() {
	return ShaderDataType::Bool2;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::bvec3>() {
	return ShaderDataType::Bool3;
}
template <> constexpr inline ShaderDataType GetShaderDataType<glm::bvec4>() {
	return ShaderDataType::Bool4;
}
#pragma endregion 

/// <summary>
/// Represents the underlying element data type of a ShaderDataType
/// </summary>
ENUM(ShaderDataTypecode, uint32_t,
	 None    = 0,
	 Float   = 0b00000001'000000'000'000,
	 Matrix  = 0b00000010'000000'000'000,
	 Int     = 0b00000100'000000'000'000,
	 Uint    = 0b00001000'000000'000'000,
	 Double  = 0b00010000'000000'000'000,
	 MatrixD = 0b00100000'000000'000'000,
	 Bool    = 0b01000000'000000'000'000,
	 Texture = 0b10000000'000000'000'000
);

/// <summary>
/// Gets the underlying ShaderDataTypecode for the given ShaderDataType
/// This is the underlying element type for the shader data type (ex, float2->float)
/// </summary>
/// <param name="type">The shader data type to examine</param>
/// <returns>The ShaderDataTypecode corresponding to the type</returns>
constexpr ShaderDataTypecode GetShaderDataTypeCode(ShaderDataType type) {
	return (ShaderDataTypecode)((uint32_t)type & ShaderDataType_TypeMask);
}

/// <summary>
/// Gets the size of the underlying ShaderDataType in bytes
/// </summary>
/// <param name="type">The type to determine the size for</param>
/// <returns>The size in bytes, or 0 if the type is invalid</returns>
constexpr uint32_t ShaderDataTypeSize(ShaderDataType type)
{
	if (type == ShaderDataType::Uint64)
		return sizeof(uint64_t);

	ShaderDataTypecode typeCode = GetShaderDataTypeCode(type);
	switch (typeCode) {
		case ShaderDataTypecode::Float:
		case ShaderDataTypecode::Int:
		case ShaderDataTypecode::Uint:
			return 4 * ((uint32_t)type & ShaderDataType_Size1Mask);
		case ShaderDataTypecode::Matrix:
			return 4 * ((uint32_t)type & ShaderDataType_Size1Mask) * (((uint32_t)type & ShaderDataType_Size2Mask) >> 3);
		case ShaderDataTypecode::Double:
			return 8 * ((uint32_t)type & ShaderDataType_Size1Mask);
		case ShaderDataTypecode::MatrixD:
			return 8 * ((uint32_t)type & ShaderDataType_Size1Mask) * (((uint32_t)type & ShaderDataType_Size2Mask) >> 3);
		case ShaderDataTypecode::Bool:
			return (uint32_t)type & ShaderDataType_Size1Mask;
		case ShaderDataTypecode::Texture:
			return 4;
		default:
			LOG_WARN("Cannot determine size for shader typecode: {}", typeCode);
			return 0;
	}
}

/// <summary>
/// Gets the number of underlying components for a given ShaderDataType
/// </summary>
/// <param name="type">The type to determine the number of components for</param>
/// <returns>The number of components in the type</returns>
constexpr uint32_t ShaderDataTypeComponentCount(ShaderDataType type)
{
	ShaderDataTypecode typeCode = (ShaderDataTypecode)((uint32_t)type & ShaderDataType_TypeMask);
	switch (typeCode) {
		case ShaderDataTypecode::Float:
		case ShaderDataTypecode::Int:
		case ShaderDataTypecode::Uint:
		case ShaderDataTypecode::Double:
		case ShaderDataTypecode::Bool:
			return (uint32_t)type & ShaderDataType_Size1Mask;
		case ShaderDataTypecode::Matrix:
		case ShaderDataTypecode::MatrixD:
			return (uint32_t)type & ShaderDataType_Size1Mask * (((uint32_t)type & ShaderDataType_Size2Mask) >> 3);
			return (uint32_t)type & ShaderDataType_Size1Mask;
		case ShaderDataTypecode::Texture:
			return 1;
		default:
			LOG_WARN(false, "Unknown ShaderDataType! {}", type);
			return 1;
	}
}

/// <summary>
/// Handles mapping a GL data type to a ShaderDataType
/// </summary>	
constexpr ShaderDataType FromGLShaderDataType(GLenum glType) {
	switch (glType) {
		case GL_FLOAT:				                        return ShaderDataType::Float;
		case GL_FLOAT_VEC2:			                        return ShaderDataType::Float2;
		case GL_FLOAT_VEC3:			                        return ShaderDataType::Float3;
		case GL_FLOAT_VEC4:			                        return ShaderDataType::Float4;
		case GL_FLOAT_MAT2:			                        return ShaderDataType::Mat2;
		case GL_FLOAT_MAT3:			                        return ShaderDataType::Mat3;
		case GL_FLOAT_MAT4:			                        return ShaderDataType::Mat4;
		case GL_FLOAT_MAT2x3:		                        return ShaderDataType::Mat2x3;
		case GL_FLOAT_MAT2x4:		                        return ShaderDataType::Mat2x4;
		case GL_FLOAT_MAT3x2:		                        return ShaderDataType::Mat3x2;
		case GL_FLOAT_MAT3x4:		                        return ShaderDataType::Mat3x4;
		case GL_FLOAT_MAT4x2:		                        return ShaderDataType::Mat4x2;
		case GL_FLOAT_MAT4x3:		                        return ShaderDataType::Mat4x3;
		case GL_INT:				                        return ShaderDataType::Int;
		case GL_INT_VEC2:			                        return ShaderDataType::Int2;
		case GL_INT_VEC3:			                        return ShaderDataType::Int3;
		case GL_INT_VEC4:			                        return ShaderDataType::Int4;
		case GL_UNSIGNED_INT:		                        return ShaderDataType::Uint;
		case GL_UNSIGNED_INT_VEC2:	                        return ShaderDataType::Uint2;
		case GL_UNSIGNED_INT_VEC3:	                        return ShaderDataType::Uint3;
		case GL_UNSIGNED_INT_VEC4:	                        return ShaderDataType::Uint4;
		case GL_UNSIGNED_INT64_ARB:	                        return ShaderDataType::Uint64;
		case GL_DOUBLE:				                        return ShaderDataType::Double;
		case GL_DOUBLE_VEC2:		                        return ShaderDataType::Double2;
		case GL_DOUBLE_VEC3:		                        return ShaderDataType::Double3;
		case GL_DOUBLE_VEC4:		                        return ShaderDataType::Double4;
		case GL_DOUBLE_MAT2:		                        return ShaderDataType::Dmat2;
		case GL_DOUBLE_MAT3:		                        return ShaderDataType::Dmat3;
		case GL_DOUBLE_MAT4:		                        return ShaderDataType::Dmat4;
		case GL_DOUBLE_MAT2x3:		                        return ShaderDataType::Dmat2x3;
		case GL_DOUBLE_MAT2x4:		                        return ShaderDataType::Dmat2x4;
		case GL_DOUBLE_MAT3x2:		                        return ShaderDataType::Dmat3x2;
		case GL_DOUBLE_MAT3x4:		                        return ShaderDataType::Dmat3x4;
		case GL_DOUBLE_MAT4x2:		                        return ShaderDataType::Dmat4x2;
		case GL_DOUBLE_MAT4x3:		                        return ShaderDataType::Dmat4x3;
		case GL_BOOL:				                        return ShaderDataType::Bool;
		case GL_BOOL_VEC2:			                        return ShaderDataType::Bool2;
		case GL_BOOL_VEC3:			                        return ShaderDataType::Bool3;
		case GL_BOOL_VEC4:			                        return ShaderDataType::Bool4;

			// Textures

		case GL_SAMPLER_1D:							        return ShaderDataType::Tex1D;
		case GL_SAMPLER_1D_ARRAY:					        return ShaderDataType::Tex1D_Array;
		case GL_SAMPLER_1D_SHADOW:					        return ShaderDataType::Tex1D_Shadow;
		case GL_SAMPLER_1D_ARRAY_SHADOW:			        return ShaderDataType::Tex1D_ShadowArray;

		case GL_SAMPLER_2D:							        return ShaderDataType::Tex2D;
		case GL_SAMPLER_2D_RECT:					        return ShaderDataType::Tex2D_Rect;
		case GL_SAMPLER_2D_RECT_SHADOW:				        return ShaderDataType::Tex2D_Rect_Shadow;
		case GL_SAMPLER_2D_ARRAY:					        return ShaderDataType::Tex2D_Array;
		case GL_SAMPLER_2D_SHADOW:					        return ShaderDataType::Tex2D_Shadow;
		case GL_SAMPLER_2D_ARRAY_SHADOW:			        return ShaderDataType::Tex2D_ShadowArray;
		case GL_SAMPLER_2D_MULTISAMPLE:				        return ShaderDataType::Tex2D_Multisample;
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:		        return ShaderDataType::Tex2D_MultisampleArray;

		case GL_SAMPLER_3D:							        return ShaderDataType::Tex3D;
		case GL_SAMPLER_CUBE:						        return ShaderDataType::TexCube;
		case GL_SAMPLER_CUBE_SHADOW:				        return ShaderDataType::TexCubeShadow;

			// Buffer Textures

		case GL_SAMPLER_BUFFER:						        return ShaderDataType::BufferTexture;
		case GL_INT_SAMPLER_BUFFER:					        return ShaderDataType::BufferTextureInt;
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:		        return ShaderDataType::BufferTextureUint;

			// Integer textures

		case GL_INT_SAMPLER_1D:						        return ShaderDataType::Tex1D_Int;
		case GL_INT_SAMPLER_1D_ARRAY:				        return ShaderDataType::Tex1D_Int_Array;
		case GL_INT_SAMPLER_2D:						        return ShaderDataType::Tex2D_Int;
		case GL_INT_SAMPLER_2D_RECT:				        return ShaderDataType::Tex2D_Int_Rect;
		case GL_INT_SAMPLER_2D_ARRAY:				        return ShaderDataType::Tex2D_Int_Array;
		case GL_INT_SAMPLER_2D_MULTISAMPLE:			        return ShaderDataType::Tex2D_Int_Multisample;
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:	        return ShaderDataType::Tex2D_Int_MultisampleArray;

		case GL_INT_SAMPLER_3D:						        return ShaderDataType::Tex3D_Int;
		case GL_INT_SAMPLER_CUBE:					        return ShaderDataType::TexCube_Int;

			// Unsigned int textures

		case GL_UNSIGNED_INT_SAMPLER_1D:					return ShaderDataType::Tex1D_Uint;
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:				return ShaderDataType::Tex1D_Uint_Array;
		case GL_UNSIGNED_INT_SAMPLER_2D:					return ShaderDataType::Tex2D_Uint;
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:				return ShaderDataType::Tex2D_Uint_Rect;
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:				return ShaderDataType::Tex2D_Uint_Array;
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:		return ShaderDataType::Tex2D_Uint_Multisample;
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:	return ShaderDataType::Tex2D_Uint_MultisampleArray;

		case GL_UNSIGNED_INT_SAMPLER_3D:					return ShaderDataType::Tex3D_Uint;
		case GL_UNSIGNED_INT_SAMPLER_CUBE:					return ShaderDataType::TexCube_Uint;

		default:                                            return ShaderDataType::None;
	}
}

/// <summary>
/// Handles mapping from a ShaderDataTypecode to an underlying GL data type
/// </summary>
constexpr GLenum ToGLElementType(ShaderDataTypecode typecode) {
	switch (typecode) {
		case ShaderDataTypecode::Float:
		case ShaderDataTypecode::Matrix:
			return GL_FLOAT;
		case ShaderDataTypecode::Int:
			return GL_INT;
		case ShaderDataTypecode::Uint:
			return GL_UNSIGNED_INT;
		case ShaderDataTypecode::Double:
		case ShaderDataTypecode::MatrixD:
			return GL_DOUBLE;
		case ShaderDataTypecode::Bool:
			return GL_BOOL;
		default:
			LOG_ASSERT(false, "Unknown Shader Data Typecode!"); return 0;
	}
}

/// <summary>
/// Represents the element type of an Index Buffer
/// </summary>
ENUM(IndexType, GLenum,
	 UByte   = GL_UNSIGNED_BYTE,
	 UShort  = GL_UNSIGNED_SHORT,
	 UInt    = GL_UNSIGNED_INT,
	 Unknown = GL_NONE
)

inline size_t GetIndexTypeSize(IndexType type) {
	switch (type) {
		case IndexType::UByte:  return sizeof(uint8_t);
		case IndexType::UShort: return sizeof(uint16_t);
		case IndexType::UInt:   return sizeof(uint32_t);
		case IndexType::Unknown:
		default:
			return 0;

	}
}

/**
 * Enumerates all possible options for glPolygonMode
 */
ENUM(FillMode, uint32_t,
	Point = GL_POINT,
	Line  = GL_LINE,
	Fill  = GL_FILL
)

/**
 * Enumerates all possible options for glCullFace, as well as
 * providing a none value that indicates culling is disabled
 */
ENUM(CullMode, uint32_t,
	None  = GL_NONE,
	Front = GL_FRONT,
	Back  = GL_BACK,
	Both  = GL_FRONT_AND_BACK
)

/**
 * Enumerates possible options for glBlendFunc 
 */
ENUM(BlendFunc, uint32_t,
	Zero             = GL_ZERO,
	One              = GL_ONE,
	SrcCol           = GL_SRC_COLOR,
	OneMinusSrcCol   = GL_ONE_MINUS_SRC_COLOR,
	DstCol           = GL_DST_COLOR,
	OneMinusDstCol   = GL_ONE_MINUS_DST_COLOR,
	SrcAlpha         = GL_SRC_ALPHA,
	OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
	DstAlpha         = GL_DST_ALPHA,
	OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
	SrcAlphaSaturate = GL_SRC_ALPHA_SATURATE
)

/**
 * Enumerates possible options for glBlendEquation 
 */
ENUM(BlendEquation, uint32_t,
	 Add        = GL_FUNC_ADD,
	 Sub        = GL_FUNC_SUBTRACT,
	 ReverseSub = GL_FUNC_REVERSE_SUBTRACT,
	 Min        = GL_MIN,
	 Max        = GL_MAX
)

/// <summary>
/// The possible options for our buffer types
/// </summary>
/// <see>https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml</see>
ENUM(BufferType, GLenum,
	Vertex  = GL_ARRAY_BUFFER,
	Index   = GL_ELEMENT_ARRAY_BUFFER,
	Uniform = GL_UNIFORM_BUFFER
)

/// <summary>
/// The possible options for our buffer usage hints
/// Stream: Contents will be modified once and used rarely
/// Static: Contents will be modified once and used regularly
/// Dynamic: Contents will be modified and used regularly
/// 
/// Draw: Content will be modified by our application and used by OpenGL
/// Read: Content will be filled with content by OpenGL to be read by our application
/// Copy: Content will be filled by OpenGL and used by other OpenGL commands (not optimized for application access)
/// </summary>
/// <see>https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml</see>
ENUM(BufferUsage, GLenum,
	StreamDraw = GL_STREAM_DRAW,
	StreamRead = GL_STREAM_READ,
	StreamCopy = GL_STREAM_COPY,

	StaticDraw = GL_STATIC_DRAW,
	StaticRead = GL_STATIC_READ,
	StaticCopy = GL_STATIC_COPY,

	DynamicDraw = GL_DYNAMIC_DRAW,
	DynamicRead = GL_DYNAMIC_READ,
	DynamicCopy = GL_DYNAMIC_COPY,
)

/// <summary>
/// We'll use this just to make it more clear what the intended usage of an attribute is in our code!
/// </summary>
ENUM(AttribUsage, uint8_t,
	 Unknown   = 0,
	 Position  = 1,
	 Color     = 2,
	 Color1    = 3,   //
	 Color2    = 4,   // Extras
	 Color3    = 5,   //
	 Texture   = 6,
	 Texture1  = 7, //
	 Texture2  = 8, // Extras
	 Texture3  = 9, //
	 Normal    = 10,
	 Tangent   = 11,
	 BiTangent = 12,
	 User0     = 13,    //
	 User1     = 14,    //
	 User2     = 15,    // Extras
	 User3     = 16     //
)

/// <summary>
/// Represents the type that a VAO attribute can have
/// </summary>
/// <see>https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml</see>
ENUM(AttributeType, GLenum,
	 Byte    = GL_BYTE,
	 UByte   = GL_UNSIGNED_BYTE,
	 Short   = GL_SHORT,
	 UShort  = GL_UNSIGNED_SHORT,
	 Int     = GL_INT,
	 UInt    = GL_UNSIGNED_INT,
	 Float   = GL_FLOAT,
	 Double  = GL_DOUBLE,
	 Unknown = GL_NONE
)

/// <summary>
/// Represents the mode in which a VAO will be drawn
/// </summary>
/// <see>https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArrays.xhtml</see>
ENUM(DrawMode, GLenum,
	 Points        = GL_POINTS,
	 LineStrip     = GL_LINE_STRIP,
	 LineLoop      = GL_LINE_LOOP,
	 LineList      = GL_LINES,
	 TriangleStrip = GL_TRIANGLE_STRIP,
	 TriangleFan   = GL_TRIANGLE_FAN,
	 TriangleList  = GL_TRIANGLES
)

/**
 * Enumerates all possible attachment options of the glFramebufferTexture and  glFramebufferRenderbuffer commands
 */
ENUM(RenderTargetAttachment, uint32_t,
	Unknown      = GL_NONE,
	Color0       = GL_COLOR_ATTACHMENT0,
	Color1       = GL_COLOR_ATTACHMENT1,
	Color2       = GL_COLOR_ATTACHMENT2,
	Color3       = GL_COLOR_ATTACHMENT3,
	Color4       = GL_COLOR_ATTACHMENT4,
	Color5       = GL_COLOR_ATTACHMENT5,
	Color6       = GL_COLOR_ATTACHMENT6,
	Color7       = GL_COLOR_ATTACHMENT7,
	Depth        = GL_DEPTH_ATTACHMENT,
	Stencil      = GL_STENCIL_ATTACHMENT,
	DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
)

/**
 * Returns true if the given attachment is a color attachment (and not depth or stencil)
 */
constexpr bool IsColorAttachment(RenderTargetAttachment attachment) {
	return attachment >= RenderTargetAttachment::Color0 && attachment <= RenderTargetAttachment::Color7;
}

/**
 * Enumerates the internal types that render targets may use (subset of texture formats)
 */
ENUM(RenderTargetType, uint32_t,
	 Unknown      = GL_NONE,
	 ColorRgba8   = GL_RGBA8,
	 ColorRgb10   = GL_RGB10,
	 ColorRgb8    = GL_RGB8,
	 ColorRG8     = GL_RG8,
	 ColorRed8    = GL_R8,
	 ColorRgb16F  = GL_RGB16F,
	 ColorRgba16F = GL_RGBA16F,
	 DepthStencil = GL_DEPTH24_STENCIL8,
	 Depth16      = GL_DEPTH_COMPONENT16,
	 Depth24      = GL_DEPTH_COMPONENT24,
	 Depth32      = GL_DEPTH_COMPONENT32,
	 Stencil4     = GL_STENCIL_INDEX4,
	 Stencil8     = GL_STENCIL_INDEX8,
	 Stencil16    = GL_STENCIL_INDEX16
)

/**
 * Enumerates the possible options for the glBindFramebuffer command
 */
ENUM_FLAGS(FramebufferBinding, GLenum,
		   None  = 0,
		   Draw  = GL_DRAW_FRAMEBUFFER,
		   Write = GL_DRAW_FRAMEBUFFER,
		   Read  = GL_READ_FRAMEBUFFER,
		   Both  = GL_FRAMEBUFFER
)

/**
 * Enumerates the possible options for glClear and glBlit commands
 */
ENUM_FLAGS(BufferFlags, GLenum,
		   None    = 0,
		   Color   = GL_COLOR_BUFFER_BIT,
		   Depth   = GL_DEPTH_BUFFER_BIT,
		   Stencil = GL_STENCIL_BUFFER_BIT,
		   All     = Color | Depth | Stencil
)