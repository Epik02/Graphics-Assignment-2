#include "Texture1D.h"
#include "Utils/Base64.h"
#include "Utils/JsonGlmHelpers.h"
#include <stb_image.h>

inline int CalcRequiredMipLevels(int size) {
	return (1 + floor(log2(size)));
}

Texture1D::Texture1D(const std::string& filePath) :
	ITexture(TextureType::_1D),
	_description(Texture1DDescription()),
	_pixelType(PixelType::Unknown)
{
	_description.Filename = filePath;
	_SetTextureParams();
	_LoadDataFromFile();
}

Texture1D::Texture1D(const Texture1DDescription& description) :
	ITexture(TextureType::_1D),
	_description(description),
	_pixelType(PixelType::Unknown)
{
	_SetTextureParams();
	if (!description.Filename.empty()) {
		_LoadDataFromFile();
	}
}

void Texture1D::SetMinFilter(MinFilter value) {
	_description.MinificationFilter = value;
	glTextureParameteri(_rendererId, GL_TEXTURE_MIN_FILTER, *_description.MinificationFilter);
}

void Texture1D::SetMagFilter(MagFilter value) {
	_description.MagnificationFilter = value;
	glTextureParameteri(_rendererId, GL_TEXTURE_MAG_FILTER, *_description.MagnificationFilter);
}

void Texture1D::SetWrap(WrapMode value) {
	_description.Wrap = value;
	glTextureParameteri(_rendererId, GL_TEXTURE_WRAP_S, *_description.Wrap);
}

void Texture1D::LoadData(uint32_t size, PixelFormat format, PixelType type, void* data, uint32_t offset /*= 0*/)
{
	LOG_ASSERT((size + offset) <= _description.Size, "Pixel bounds are outside of the X extents of the image!");

	_description.FormatHint = format;
	_pixelType = type;

	// Align the data store to the size of a single component to ensure we don't get weirdness with images that aren't RGBA
	// See https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPixelStore.xhtml
	int componentSize = (GLint)GetTexelComponentSize(type);
	glPixelStorei(GL_PACK_ALIGNMENT, componentSize);

	// Upload our data to our image
	glTextureSubImage1D(_rendererId, 0, offset, size, (GLenum)format, (GLenum)type, data);

	// If requested, generate mip-maps for our texture
	if (_description.GenerateMipMaps) {
		glGenerateTextureMipmap(_rendererId);
	}
}

nlohmann::json Texture1D::ToJson() const
{
	nlohmann::json result = {
		{ "wrap_mode",  ~_description.Wrap },
		{ "filter_min",       ~_description.MinificationFilter },
		{ "filter_mag",       ~_description.MagnificationFilter },
		{ "generate_mipmaps",  _description.GenerateMipMaps },
	};

	if (!_description.Filename.empty()) {
		result["filename"] = _description.Filename;
	}
	else if (_pixelType != PixelType::Unknown) {
		result["size"] = _description.Size;
		result["format"] = ~_description.FormatHint;
		result["pixel_type"] = ~_pixelType;

		if (_description.Size > 0 && _description.FormatHint != PixelFormat::Unknown) {
			size_t dataSize = GetTexelSize(_description.FormatHint, _pixelType) * _description.Size;
			uint8_t* dataStore = new uint8_t[dataSize];
			glGetTextureImage(_rendererId, 0, *_description.Format, *_pixelType, dataSize, dataStore);
			result["data"] = Base64::Encode(dataStore, dataSize);
		}
	}
	return result;
}

Texture1D::Sptr Texture1D::FromJson(const nlohmann::json& data)
{
	Texture1DDescription description = Texture1DDescription();
	description.Filename = JsonGet<std::string>(data, "filename", "");
	description.Size = JsonGet(data, "size", 0);
	description.Wrap = JsonParseEnum(WrapMode, data, "wrap_mode", WrapMode::MirrorClampToEdge);
	description.MinificationFilter = JsonParseEnum(MinFilter, data, "filter_min", MinFilter::NearestMipNearest);
	description.MagnificationFilter = JsonParseEnum(MagFilter, data, "filter_mag", MagFilter::Linear);
	description.GenerateMipMaps = JsonGet(data, "generate_mipmaps", false);
	description.FormatHint = JsonParseEnum(PixelFormat, data, "format", PixelFormat::Unknown);

	Texture1D::Sptr result = std::make_shared<Texture1D>(description);

	// If we embedded data into the JSON, load it now
	if (description.Filename.empty() && data.contains("data") && data["data"].is_string()) {
		PixelType type = JsonParseEnum(PixelType, data, "pixel_type", PixelType::Unknown);
		try {
			std::string rawData = Base64::Decode(data['data'].get<std::string>());
			result->LoadData(description.Size, description.FormatHint, type, rawData.data());
		}
		catch (std::runtime_error()) {
			LOG_WARN("JSON blob had data, but failed to load to texture");
		}
	}

	return result;
}

void Texture1D::_LoadDataFromFile()
{
	LOG_ASSERT(_description.Size == 0, "This texture has already been configured with a size! Cannot re-allocate memory!");

	if (!_description.Filename.empty()) {
		// Variables that will store properties about our image
		int width, height, numChannels;
		const int targetChannels = GetTexelComponentCount(_description.FormatHint);

		// Use STBI to load the image
		stbi_set_flip_vertically_on_load(true);
		uint8_t* data = stbi_load(_description.Filename.c_str(), &width, &height, &numChannels, targetChannels);

		// If we could not load any data, warn and return null
		if (data == nullptr) {
			LOG_WARN("STBI Failed to load image from \"{}\"", _description.Filename);
			return;
		}

		// We should estimate a good format for our data

		// numChannels will store the number of channels in the image on disk, if we overrode that we should use the override value
		if (targetChannels != 0)
			numChannels = targetChannels;

		// We'll determine a recommended format for the image based on number of channels
		// We hinted that we wanted a certain number of channels, but we're not guaranteed
		// that all those channels exist (ex: loading an RGB image but requesting RGBA)
		InternalFormat internal_format = GetInternalFormatForChannels8(numChannels);
		PixelFormat    image_format = GetPixelFormatForChannels(numChannels);

		// This is one of those poorly documented things in OpenGL
		if ((numChannels * width) % 4 != 0) {
			LOG_WARN("The alignment of a horizontal line is not a multiple of 4, this will require a call to glPixelStorei(GL_PACK_ALIGNMENT)");
		}

		// Update our description to match what we loaded
		_description.Format = internal_format;
		_description.Size = width * height;

		// Allocates our memory
		_SetTextureParams();

		// Upload data to our texture
		LoadData(width * height, image_format, PixelType::UByte, data);

		// We now have data in the image, we can clear the STBI data
		stbi_image_free(data);
	}

	SetDebugName(_description.Filename);
}

void Texture1D::_SetTextureParams()
{
	// Calculate how many layers of storage to allocate based on whether mipmaps are enabled or not
	int layers = _description.GenerateMipMaps ? CalcRequiredMipLevels(_description.Size) : 1;
	// Allocates the memory for our texture
	glTextureStorage1D(_rendererId, layers, (GLenum)_description.Format, _description.Size);

	glTextureParameteri(_rendererId, GL_TEXTURE_MIN_FILTER, (GLenum)_description.MinificationFilter);
	glTextureParameteri(_rendererId, GL_TEXTURE_MAG_FILTER, (GLenum)_description.MagnificationFilter);
	glTextureParameteri(_rendererId, GL_TEXTURE_WRAP_S, (GLenum)_description.Wrap);
}

Texture1D::Sptr Texture1D::LoadFromFile(const std::string& path, const Texture1DDescription& description /*= Texture1DDescription()*/, bool forceRgba /*= true*/)
{
	// Create a copy of the description and change filename to the path
	Texture1DDescription desc = description;
	desc.Filename = path;

	// Create a texture from the description (it'll load the file)
	Texture1D::Sptr result = std::make_shared<Texture1D>(desc);

	return result;
}
