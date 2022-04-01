#include "Texture2D.h"
#include <stb_image.h>
#include <Logging.h>
#include "GLM/glm.hpp"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/Base64.h"

/// <summary>
/// Get the number of mipmap levels required for a texture of the given size
/// </summary>
/// <param name="width">The width of the texture in pixels</param>
/// <param name="height">The height of the texture in pixels</param>
/// <returns>Number of mip levels required for the texture</returns>
inline int CalcRequiredMipLevels(int width, int height) {
	return (1 + floor(log2(glm::max(width, height))));
}

nlohmann::json Texture2D::ToJson() const {
	nlohmann::json result = {
		{ "wrap_s",  ~_description.HorizontalWrap },
		{ "wrap_t",  ~_description.VerticalWrap },
		{ "filter_min",       ~_description.MinificationFilter },
		{ "filter_mag",       ~_description.MagnificationFilter },
		{ "anisotropic",       _description.MaxAnisotropic },
		{ "generate_mipmaps",  _description.GenerateMipMaps },
	};

	if (!_description.Filename.empty()) {
		result["filename"] = _description.Filename;
	}
	else if (_pixelType != PixelType::Unknown) {
		result["size_x"] = _description.Width;
		result["size_y"] = _description.Width;

		result["format"] = ~_description.FormatHint;
		result["pixel_type"] = ~_pixelType;
		if (_description.Width * _description.Height > 0 && _description.FormatHint != PixelFormat::Unknown) {
			size_t dataSize = GetTexelSize(_description.FormatHint, _pixelType) * _description.Width * _description.Height;
			uint8_t* dataStore = new uint8_t[dataSize];
			glGetTextureImage(_rendererId, 0, *_description.Format, *_pixelType, dataSize, dataStore);
			result["data"] = Base64::Encode(dataStore, dataSize);
		}
	}

	return result;
}

Texture2D::Sptr Texture2D::FromJson(const nlohmann::json& data)
{
	Texture2DDescription descr = Texture2DDescription();
	descr.Filename = data["filename"];
	descr.HorizontalWrap = JsonParseEnum(WrapMode, data, "wrap_s", WrapMode::ClampToEdge);
	descr.VerticalWrap   = JsonParseEnum(WrapMode, data, "wrap_t", WrapMode::ClampToEdge);
	descr.MinificationFilter  = JsonParseEnum(MinFilter, data, "filter_min", MinFilter::NearestMipNearest);
	descr.MagnificationFilter = JsonParseEnum(MagFilter, data, "filter_mag", MagFilter::Linear);
	descr.MaxAnisotropic      = JsonGet(data, "anisotropic", 0.0f);
	descr.GenerateMipMaps     = JsonGet(data, "generate_mipmaps", false);

	Texture2D::Sptr result = std::make_shared<Texture2D>(descr);

	// If we embedded data into the JSON, load it now
	if (descr.Filename.empty() && data.contains("data") && data["data"].is_string()) {
		PixelType type = JsonParseEnum(PixelType, data, "pixel_type", PixelType::Unknown);
		try {
			std::string rawData = Base64::Decode(data['data'].get<std::string>());
			result->LoadData(descr.Width, descr.Height, descr.FormatHint, type, rawData.data());
		}
		catch (std::runtime_error()) {
			LOG_WARN("JSON blob had data, but failed to load to texture");
		}
	}

	return result;
}

Texture2D::Texture2D(const Texture2DDescription& description) : 
	ITexture(TextureType::_2D),
	_description(description),
	_pixelType(PixelType::Unknown)
{
	_SetTextureParams();
	if (!description.Filename.empty()) {
		_LoadDataFromFile();
	}
}

Texture2D::Texture2D(const std::string& filePath) : 
	ITexture(TextureType::_2D),
	_description(Texture2DDescription()),
	_pixelType(PixelType::Unknown)
{
	_description.Filename = filePath;
	_SetTextureParams();
	_LoadDataFromFile();
}

void Texture2D::SetMinFilter(MinFilter value) {
	if (_description.MultisampleCount == 1) {
		_description.MinificationFilter = value;
		glTextureParameteri(_rendererId, GL_TEXTURE_MIN_FILTER, *_description.MinificationFilter);
	}
	else {
		LOG_WARN("Attempted to set minification filter on a multisampled texture, ignoring");
	}
}

void Texture2D::SetMagFilter(MagFilter value) {
	if (_description.MultisampleCount == 1) {
		_description.MagnificationFilter = value;
		glTextureParameteri(_rendererId, GL_TEXTURE_MAG_FILTER, *_description.MagnificationFilter);
	} else {
		LOG_WARN("Attempted to set magnification filter on a multisampled texture, ignoring");
	}
}

void Texture2D::SetAnisoLevel(float value) {
	if (value != _description.MaxAnisotropic) {
		_description.MaxAnisotropic = glm::clamp(value, 1.0f, ITexture::GetLimits().MAX_ANISOTROPY);
		glTextureParameterf(_rendererId, GL_TEXTURE_MAX_ANISOTROPY, _description.MaxAnisotropic);

		if (_description.GenerateMipMaps) {
			glGenerateTextureMipmap(_rendererId);
		}
	}
}

void Texture2D::LoadData(uint32_t width, uint32_t height, PixelFormat format, PixelType type, void* data, uint32_t offsetX, uint32_t offsetY) {
	// Ensure the rectangle we're setting is within the bounds of the image
	LOG_ASSERT((width + offsetX) <= _description.Width, "Pixel bounds are outside of the X extents of the image!");
	LOG_ASSERT((height + offsetY) <= _description.Height, "Pixel bounds are outside of the Y extents of the image!");

	_description.FormatHint = format;
	_pixelType = type;

	// Align the data store to the size of a single component to ensure we don't get weirdness with images that aren't RGBA
	// See https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPixelStore.xhtml
	int componentSize = (GLint)GetTexelComponentSize(type);
	glPixelStorei(GL_PACK_ALIGNMENT, componentSize);

	// Upload our data to our image
	glTextureSubImage2D(_rendererId, 0, offsetX, offsetY, width, height, (GLenum)format, (GLenum)type, data);

	// If requested, generate mip-maps for our texture
	if (_description.GenerateMipMaps) {
		glGenerateTextureMipmap(_rendererId);
	}
}

void Texture2D::_LoadDataFromFile() {
	LOG_ASSERT(_description.Width + _description.Height == 0, "This texture has already been configured with a size! Cannot re-allocate memory!");

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
			return ;
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
		_description.Width = width;
		_description.Height = height;

		// Allocates our memory
		_SetTextureParams();

		// Upload data to our texture
		LoadData(width, height, image_format, PixelType::UByte, data);

		// We now have data in the image, we can clear the STBI data
		stbi_image_free(data);
	}
	
	SetDebugName(_description.Filename);
}

void Texture2D::_SetTextureParams() {
	// If we have a multisampled texture, and the current type is 2D, change it to 2D multisampled
	if (_description.MultisampleCount > 1 && _type == TextureType::_2D) {
		glDeleteTextures(1, &_rendererId);
		_type = TextureType::_2DMultisample;
		glCreateTextures(*_type, 1, &_rendererId);
	}

	// If the anisotropy is negative, we assume that we want max anisotropy
	if (_description.MaxAnisotropic < 0.0f) {
		_description.MaxAnisotropic = ITexture::GetLimits().MAX_ANISOTROPY;
	}

	// Make sure the size is greater than zero and that we have a format specified before trying to set parameters
	if ((_description.Width * _description.Height > 0) && _description.Format != InternalFormat::Unknown) {
		// If the texture is NOT multisampled, we proceed as normal
		if (_description.MultisampleCount == 1) {
			// Calculate how many layers of storage to allocate based on whether mipmaps are enabled or not
			int layers = _description.GenerateMipMaps ? CalcRequiredMipLevels(_description.Width, _description.Height) : 1;
			// Allocates the memory for our texture
			glTextureStorage2D(_rendererId, layers, (GLenum)_description.Format, _description.Width, _description.Height);

			glTextureParameteri(_rendererId, GL_TEXTURE_MIN_FILTER, (GLenum)_description.MinificationFilter);
			glTextureParameteri(_rendererId, GL_TEXTURE_MAG_FILTER, (GLenum)_description.MagnificationFilter);
			glTextureParameterf(_rendererId, GL_TEXTURE_MAX_ANISOTROPY, _description.MaxAnisotropic);
		}
		// Texture is multisampled, we need to allocate memory differently
		else {
			glTextureStorage2DMultisample(_rendererId, _description.MultisampleCount, *_description.Format, _description.Width, _description.Height, true);
		}

		glTextureParameteri(_rendererId, GL_TEXTURE_WRAP_S, (GLenum)_description.HorizontalWrap);
		glTextureParameteri(_rendererId, GL_TEXTURE_WRAP_T, (GLenum)_description.VerticalWrap);

		glTextureParameteri(_rendererId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(_rendererId, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
}

Texture2D::Sptr Texture2D::LoadFromFile(const std::string& path, const Texture2DDescription& description, bool forceRgba) {
	// Create a copy of the description and change filename to the path
	Texture2DDescription desc = description;
	desc.Filename = path;

	// Create a texture from the description (it'll load the file)
	Texture2D::Sptr result = std::make_shared<Texture2D>(desc);

	return result;
}