#pragma once
#include <EnumToString.h>
#include "ITexture.h"

/*
0 	GL_TEXTURE_CUBE_MAP_POSITIVE_X
1 	GL_TEXTURE_CUBE_MAP_NEGATIVE_X
2 	GL_TEXTURE_CUBE_MAP_POSITIVE_Y
3 	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
4 	GL_TEXTURE_CUBE_MAP_POSITIVE_Z
5 	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	*/
ENUM(CubeMapFace, GLint,
	Unknown = -1,
	PosX = 0,
	NegX = 1,
	PosY = 2,
	NegY = 3,
	PosZ = 4,
	NegZ = 5
);

/// <summary>
/// Describes the parameters required to generate a texture cubemap
/// </summary>
struct TextureCubeDescription {
	/// <summary>
	/// The number of texels in this image along the x and y axes
	/// Note that cubemaps require square textures for each face
	/// </summary>
	uint32_t       Size;
	/// <summary>
	/// The internal format that OpenGL should use when storing this texture
	/// </summary>
	InternalFormat Format;
	/// <summary>
	/// The filter to use when multiple texels will map to a single pixel
	/// </summary>
	MinFilter      MinificationFilter;
	/// <summary>
	/// The filter to use when one texel will map to multiple pixels
	/// </summary>
	MagFilter      MagnificationFilter;

	/// <summary>
	/// The base filename to load all cubemap faces from, will select files
	/// for each face with the format "Filename_Face.ext" (ex: "Skybox_NegX.png")
	/// </summary>
	std::string    Filename;

	/// <summary>
	/// The path to the source file for each face of the cubemap
	/// </summary>
	std::unordered_map<CubeMapFace, std::string> FaceFileNames;

	/// <summary>
	/// Used as a hint for loading texture from files, determines
	/// the number of channels, default RGBA. Only used to determine
	/// channel count
	/// </summary>
	PixelFormat    FormatHint;

	/// <summary>
	/// Creates a default (empty) cubemap description
	/// </summary>
	TextureCubeDescription() :
		Size(0),
		Format(InternalFormat::Unknown),
		MinificationFilter(MinFilter::NearestMipLinear),
		MagnificationFilter(MagFilter::Linear),
		Filename(""),
		FormatHint(PixelFormat::RGBA)
	{ }
};

class TextureCube : public ITexture {
public:
	DEFINE_RESOURCE(TextureCube);

	// Make sure we mark our destructor as virtual so base class is called
	virtual ~TextureCube() = default;

public:
	TextureCube(const std::string& baseFilename);
	TextureCube(const std::unordered_map<CubeMapFace, std::string>& faceFilenames);
	TextureCube(const TextureCubeDescription& description);

	/// <summary>
	/// Gets the width of this texture in pixels
	/// </summary>
	uint32_t GetFaceSize() const { return _description.Size; }
	/// <summary>
	/// Gets the internal format OpenGL is using for this texture
	/// </summary>
	InternalFormat GetFormat() const { return _description.Format; }
	/// <summary>
	/// Gets the minification filter that the texture is using
	/// </summary>
	MinFilter GetMinFilter() const { return _description.MinificationFilter; }
	/// <summary>
	/// Gets the magnification filter that the texture is using
	/// </summary>
	MagFilter GetMagFilter() const { return _description.MagnificationFilter; }

	/// <summary>
	/// Gets this texture's description, which contains basic information about the
	/// texture's dimensions and creation parameters
	/// </summary>
	const TextureCubeDescription& GetDescription() const { return _description; }

	virtual nlohmann::json ToJson() const override;
	static TextureCube::Sptr FromJson(const nlohmann::json& data);

protected:
	TextureCubeDescription _description;

	virtual void _LoadFromDescription();
	virtual void _LoadImages(const std::unordered_map<CubeMapFace, std::string>& faceFilenames);

	/// <summary>
	/// Allocates our texture's memory and sets sampling / filtering parameters
	/// </summary>
	void _SetTextureParams();
};