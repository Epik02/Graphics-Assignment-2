#pragma once
#include "ITexture.h"

/// <summary>
/// Describes all parameters we can manipulate with our 2D Textures
/// </summary>
struct Texture1DDescription {
	/// <summary>
	/// The number of texels in this image
	/// </summary>
	uint32_t       Size;
	/// <summary>
	/// The internal format that OpenGL should use when storing this texture
	/// </summary>
	InternalFormat Format;
	/// <summary>
	/// The wrap mode to use when a UV coordinate is outside the 0-1 range on the x axis
	/// </summary>
	WrapMode       Wrap;
	/// <summary>
	/// The filter to use when multiple texels will map to a single pixel
	/// </summary>
	MinFilter      MinificationFilter;
	/// <summary>
	/// The filter to use when one texel will map to multiple pixels
	/// </summary>
	MagFilter      MagnificationFilter;
	/// <summary>
	/// True if this texture should generate mip maps (smaller copies of the image with filtering pre-applied)
	/// </summary>
	bool           GenerateMipMaps;

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

	Texture1DDescription() :
		Size(0),
		Format(InternalFormat::Unknown),
		Wrap(WrapMode::Repeat),
		MinificationFilter(MinFilter::NearestMipLinear),
		MagnificationFilter(MagFilter::Linear),
		GenerateMipMaps(true),
		Filename(""),
		FormatHint(PixelFormat::RGBA)
	{ }
};

class Texture1D : public ITexture {
public:
	DEFINE_RESOURCE(Texture1D)

		// Make sure we mark our destructor as virtual so base class is called
		virtual ~Texture1D() = default;

public:
	Texture1D(const std::string& filePath);
	Texture1D(const Texture1DDescription& description);

	/// <summary>
	/// Gets the internal format OpenGL is using for this texture
	/// </summary>
	InternalFormat GetFormat() const { return _description.Format; }
	/// <summary>
	/// Gets the width of this texture in pixels
	/// </summary>
	uint32_t GetSize() const { return _description.Size; }
	/// <summary>
	/// Gets the sampler wrap mode along the x/s/u axis for this texture
	/// </summary>
	WrapMode GetWrap() const { return _description.Wrap; }

	/// <summary>
	/// Gets the minification filter that the texture is using
	/// </summary>
	MinFilter GetMinFilter() const { return _description.MinificationFilter; }
	void SetMinFilter(MinFilter value);
	/// <summary>
	/// Gets the magnification filter that the texture is using
	/// </summary>
	MagFilter GetMagFilter() const { return _description.MagnificationFilter; }
	void SetMagFilter(MagFilter value);

	void SetWrap(WrapMode value);

	/// <summary>
	/// Loads a region of data into this texture
	/// Bounds must be contained by the bounds of the texture
	/// format and type must be convertible to the texture's internal format
	/// </summary>
	/// <param name="size">The size of the data frame, in pixels</param>
	/// <param name="format">The pixel layout of the data</param>
	/// <param name="type">The pixel base type of the data</param>
	/// <param name="data">A pointer to the data to load into this texture</param>
	/// <param name="offset">The offset into the texture to load the data, in pixels</param>
	void LoadData(uint32_t size, PixelFormat format, PixelType type, void* data, uint32_t offset = 0);

	/// <summary>
	/// Gets this texture's description, which contains basic information about the
	/// texture's dimensions and creation parameters
	/// </summary>
	const Texture1DDescription& GetDescription() const { return _description; }

	virtual nlohmann::json ToJson() const override;
	static Texture1D::Sptr FromJson(const nlohmann::json& data);

protected:
	Texture1DDescription _description;
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
	static Texture1D::Sptr LoadFromFile(const std::string& path, const Texture1DDescription& description = Texture1DDescription(), bool forceRgba = true);
};