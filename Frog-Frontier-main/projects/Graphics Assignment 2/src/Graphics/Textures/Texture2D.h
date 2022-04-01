#pragma once
#include "ITexture.h"

/// <summary>
/// Describes all parameters we can manipulate with our 2D Textures
/// </summary>
struct Texture2DDescription {
	/// <summary>
	/// The number of texels in this image along the x axis
	/// </summary>
	uint32_t       Width;
	/// <summary>
	/// The number of texels in this image along the y axis
	/// </summary>
	uint32_t       Height;
	/// <summary>
	/// The internal format that OpenGL should use when storing this texture
	/// </summary>
	InternalFormat Format;
	/// <summary>
	/// The wrap mode to use when a UV coordinate is outside the 0-1 range on the x axis
	/// </summary>
	WrapMode       HorizontalWrap;
	/// <summary>
	/// The wrap mode to use when a UV coordinate is outside the 0-1 range on the y axis
	/// </summary>
	WrapMode       VerticalWrap;
	/// <summary>
	/// The filter to use when multiple texels will map to a single pixel
	/// </summary>
	MinFilter      MinificationFilter;
	/// <summary>
	/// The filter to use when one texel will map to multiple pixels
	/// </summary>
	MagFilter      MagnificationFilter;
	/// <summary>
	/// The level of anisotropic filtering to use when this texture is viewed at an oblique angle
	/// </summary>
	/// <see>https://en.wikipedia.org/wiki/Anisotropic_filtering</see>
	float          MaxAnisotropic;
	/// <summary>
	/// True if this texture should generate mip maps (smaller copies of the image with filtering pre-applied)
	/// </summary>
	bool           GenerateMipMaps;
	/// <summary>
	/// Returns the number of samples if the texture is multisampled, default 1
	/// </summary>
	uint8_t        MultisampleCount;

	/// <summary>
	/// The path to the source file for the image, or an empty string if the file has been
	/// generated
	/// </summary>
	std::string    Filename;

	/// <summary>
	/// Used as a hint for loading texture from files, determines
	/// the number of channels, default RGBA. Only used to determine
	/// channel count
	/// </summary>
	PixelFormat    FormatHint;

	Texture2DDescription() :
		Width(0), Height(0),
		Format(InternalFormat::Unknown),
		HorizontalWrap(WrapMode::Repeat),
		VerticalWrap(WrapMode::Repeat),
		MinificationFilter(MinFilter::NearestMipLinear),
		MagnificationFilter(MagFilter::Linear),
		MaxAnisotropic(-1.0f), // max aniso by default
		GenerateMipMaps(true),
		MultisampleCount(1),
		Filename(""),
		FormatHint(PixelFormat::RGBA)
	{ }
};

class Texture2D : public ITexture {
public:
	DEFINE_RESOURCE(Texture2D)

	// Make sure we mark our destructor as virtual so base class is called
	virtual ~Texture2D() = default;

public:
	Texture2D(const std::string& filePath);
	Texture2D(const Texture2DDescription& description);

	/// <summary>
	/// Gets the internal format OpenGL is using for this texture
	/// </summary>
	InternalFormat GetFormat() const { return _description.Format; }
	/// <summary>
	/// Gets the width of this texture in pixels
	/// </summary>
	uint32_t GetWidth() const { return _description.Width; }
	/// <summary>
	/// Gets the height of this texture in pixels
	/// </summary>
	uint32_t GetHeight() const { return _description.Height; }
	/// <summary>
	/// Gets the sampler wrap mode along the x/s/u axis for this texture
	/// </summary>
	WrapMode GetWrapS() const { return _description.HorizontalWrap; }
	/// <summary>
	/// Gets the sampler wrap mode along the y/t/v axis for this texture
	/// </summary>
	WrapMode GetWrapT() const { return _description.VerticalWrap; }

	/// <summary>
	/// Gets the minification filter that the texture is using
	/// </summary>
	MinFilter GetMinFilter() const { return _description.MultisampleCount == 1 ? _description.MinificationFilter : MinFilter::Unknown; }
	void SetMinFilter(MinFilter value);
	/// <summary>
	/// Gets the magnification filter that the texture is using
	/// </summary>
	MagFilter GetMagFilter() const { return _description.MultisampleCount == 1 ? _description.MagnificationFilter : MagFilter::Unknown; }
	void SetMagFilter(MagFilter value);

	float GetAnisoLevel() const { return _description.MaxAnisotropic; }
	void SetAnisoLevel(float value);

	/// <summary>
	/// Loads a region of data into this texture
	/// Bounds must be contained by the bounds of the texture
	/// format and type must be convertible to the texture's internal format
	/// </summary>
	/// <param name="width">The width of the data frame, in pixels</param>
	/// <param name="height">The height of the data frame, in pixels</param>
	/// <param name="format">The pixel layout of the data</param>
	/// <param name="type">The pixel base type of the data</param>
	/// <param name="data">A pointer to the data to load into this texture</param>
	/// <param name="offsetX">The x edge of the destination rectangle in the texture, left->right</param>
	/// <param name="offsetY">The y edge of the destination rectangle in the texture, bottom->top</param>
	void LoadData(uint32_t width, uint32_t height, PixelFormat format, PixelType type, void* data, uint32_t offsetX = 0, uint32_t offsetY = 0);

	/// <summary>
	/// Gets this texture's description, which contains basic information about the
	/// texture's dimensions and creation parameters
	/// </summary>
	const Texture2DDescription& GetDescription() const { return _description; }

	virtual nlohmann::json ToJson() const override;
	static Texture2D::Sptr FromJson(const nlohmann::json& data);

protected:
	Texture2DDescription _description;
	PixelType _pixelType;

	/// <summary>
	/// Loads this texture from the file specified in the description
	/// Will overwrite description size
	/// </summary>
	void _LoadDataFromFile();
	/// <summary>
	/// Allocates our texture's memory and sets sampling / filtering parameters
	/// </summary>
	void _SetTextureParams();

public:
	static Texture2D::Sptr LoadFromFile(const std::string& path, const Texture2DDescription& description = Texture2DDescription(), bool forceRgba = true);
};
