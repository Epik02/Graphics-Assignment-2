#include "Graphics/Font.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include <set>
#include <codecvt>
#include <locale>
#include <cstdint>
#include <stb_rect_pack.h>
#include "Utils/JsonGlmHelpers.h"

#define OVERSAMPLE_X 1
#define OVERSAMPLE_Y 1
#define PADDING 1

Font::Font() : Font("", 0.0f) { }

Font::Font(const std::string& fontPath, float size) :
	IResource(),
	_fontPath(fontPath),
	_fontSize(size),
	_glyphs(nullptr),
	_atlas(nullptr),
	_ascent(0),
	_descent(0),
	_lineGap(0.0f),
	_emToPixel(0.0f),
	_pixelHeightScale(0.0f),
	_fontInfo(stbtt_fontinfo()),
	_defaultGlyph(GlyphInfo()),
	_atlasWidth(256),
	_atlasHeight(256)
{
	// For the box character
	_glyphRanges.push_back({ 0xE000u, 0xE000u });
	// Default ASCII characters
	_glyphRanges.push_back({ 1, 255 });

	if (!fontPath.empty()) {
		Load(fontPath, size);
	}
}

Font::~Font() {
	delete[] _glyphs;
	_atlas = nullptr;
}

void Font::Load(const std::string& fontPath, float size /*= 16.0f*/)
{
	// Read the contents of the file
	std::string data = FileHelpers::ReadFile(fontPath);

	// Make sure we got some data
	if (!data.empty()) {
		_fontPath = fontPath;
		_fontData = data;

		if (_glyphs != nullptr) {
			delete[] _glyphs;
			_glyphs = nullptr;
		}
		_atlas = nullptr;

		uint8_t* rawData = reinterpret_cast<uint8_t*>(_fontData.data());

		// Attempt to initialize the font from the data read from the file
		if (!stbtt_InitFont(&_fontInfo, rawData, 0)) {
			LOG_ERROR("Failed to initialize font");
			return;
		}

		// Gets the font metrics
		stbtt_GetFontVMetrics(&_fontInfo, &_ascent, &_descent, &_lineGap);
		_pixelHeightScale = stbtt_ScaleForPixelHeight(&_fontInfo, _fontSize);
		_emToPixel        = stbtt_ScaleForMappingEmToPixels(&_fontInfo, _fontSize);
	} else {
		LOG_ERROR("Failed to load font file from {}", fontPath);
	}
}

void Font::AddGlyphRange(uint32_t min, uint32_t max) {
	LOG_ASSERT(_atlas == nullptr, "Cannot add glyphs after the font has been baked!");
	_glyphRanges.push_back({ min, max });
}

void Font::Bake() {
	LOG_ASSERT(_atlas == nullptr, "Bake has already been called!");
	LOG_ASSERT(_fontInfo.data != nullptr, "Have not loaded a font asset!");

	uint8_t* rawFontData = reinterpret_cast<uint8_t*>(_fontData.data());

	// Collect all codepoint ranges into a set, so we have a list of unique codepoints
	std::set<int> codePoints;
	uint32_t numCodepoints = 0;
	for (const auto& range : _glyphRanges) {
		for (uint32_t ix = range.x; ix <= range.y; ix++) {
			// skip if the font doesn't have that glyph
			if (!stbtt_FindGlyphIndex(&_fontInfo, ix)) {
				continue;
			}
			numCodepoints += codePoints.emplace(ix).second ? 1 : 0;
		}
	}

	// Allocate our glyph data for the number of unicode character's we're supporting
	_glyphs = new stbtt_packedchar[numCodepoints];
	memset(_glyphs, 0, sizeof(stbtt_packedchar) * numCodepoints);

	// Collect unicode ranges, may differ from input ranges!
	std::vector<stbtt_pack_range> ranges;

	// Create the initial data structure, we'll store copies of this as we go
	stbtt_pack_range current;
	current.font_size = _fontSize;
	current.first_unicode_codepoint_in_range = *codePoints.begin();
	current.num_chars = 0;
	current.chardata_for_range = _glyphs;
	current.h_oversample = OVERSAMPLE_X;
	current.v_oversample = OVERSAMPLE_Y;
	current.array_of_unicode_codepoints = (int*)(&*codePoints.begin());

	// Calculate surface area of glyphs as we go
	int totalSurface = 0;
	// We track number of encoded characters, as well as the previous processed codepoint
	// to check for jumps in the range
	uint32_t prevCodePoint = *codePoints.begin() - 1;
	uint32_t encodedChars = 0;

	// Iterate over the unique codepoint set (which is sorted!)
	for (uint32_t codepoint : codePoints) {

		// We have a break in the codepoints, start a new range!
		if (codepoint - 1 != prevCodePoint) {
			// Track the end of the current range and store it
			current.num_chars = prevCodePoint - current.first_unicode_codepoint_in_range + 1;
			ranges.push_back(current);

			// Start the next range
			current.first_unicode_codepoint_in_range = codepoint;
			current.chardata_for_range = _glyphs + encodedChars;
			current.array_of_unicode_codepoints = (int*)(&*codePoints.begin()) + encodedChars;
		}

		// We have another character
		encodedChars++;

		// Track the previous unicode character
		prevCodePoint = codepoint;
	}

	// We've processed all codepoints, finish the current range and store it
	current.num_chars = prevCodePoint - current.first_unicode_codepoint_in_range + 1;
	ranges.push_back(current);

	_CrtCheckMemory();

	// Create a texture to store the atlas
	Texture2DDescription desc;
	desc.Width = _atlasWidth;
	desc.Height = _atlasHeight;
	desc.Format = InternalFormat::R8;
	_atlas = std::make_shared<Texture2D>(desc);

	// Allocate memory for the image, and point rect pack at it
	uint8_t* atlasData = new uint8_t[desc.Width * (size_t)desc.Height];
	memset(atlasData, 0, desc.Width * desc.Height);

	stbtt_pack_context context;
	if (!stbtt_PackBegin(&context, atlasData, _atlasWidth, _atlasHeight, 0, 1, nullptr)) {
		LOG_ERROR("Failed to pack font texture");
		delete[] atlasData;
		return;
	}
	_CrtCheckMemory();

	stbtt_PackSetOversampling(&context, OVERSAMPLE_X, OVERSAMPLE_Y);
	for (auto& range : ranges) {
		if (!stbtt_PackFontRange(&context, rawFontData, 0, range.font_size, range.first_unicode_codepoint_in_range, range.num_chars, range.chardata_for_range)) {
			LOG_ERROR("Failed to pack font range");
			delete[] atlasData;
			return;
		}
		_CrtCheckMemory();
	}
	stbtt_PackEnd(&context);

	_CrtCheckMemory();

	// Upload data into the image
	_atlas->LoadData(desc.Width, desc.Height, PixelFormat::Red, PixelType::UByte, atlasData);
	delete[] atlasData;

	uint32_t index = 0;
	for (uint32_t codepoint : codePoints) {
		_glyphMap[codepoint] = __CreateGlyph(index);
		index++;

		if (codepoint == 0xE000u)
			_defaultGlyph = _glyphMap[codepoint];
	}
}

const Texture2D::Sptr& Font::GetAtlas() {
	return _atlas;
}

GlyphInfo Font::GetGlyph(uint32_t codePoint, float offsetX, float offsetY) const {
	// Try and get glyph info from the codepoint, otherwise grab the default glyph
	auto it = _glyphMap.find(codePoint);		
	GlyphInfo result = it == _glyphMap.end() ? _defaultGlyph : it->second;

	result.OffsetX += offsetX;
	result.OffsetY += offsetY;

	return result;
}

float Font::GetKerning(int char1, int char2) const {
	return stbtt_GetCodepointKernAdvance(&_fontInfo, char1, char2) * _pixelHeightScale;
}

float Font::GetLineHeight() const {
	return (_ascent - _descent + _lineGap) * _pixelHeightScale;
}

glm::vec2 Font::MeausureString(const std::string& text, const float scale /*= 1.0f*/) {
	// We can convert an ASCII string to unicode!
	std::wstring unicode = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(text);
	return MeausureString(unicode, scale);
}

glm::vec2 Font::MeausureString(const std::wstring& text, const float scale /*= 1.0f*/) {
	// Will cache the current glyph
	GlyphInfo glyph;

	// We'll track the position and max size of the text
	float xOff{ 0 }, yOff{ 0 };
	float lineHeight = 0.0f;
	float maxWidth = 0.0f;
	float totalHeight = 0.0f;

	// Iterate over all characters, ascii and unicode overlap in the 0-255 range!
	for (size_t i = 0; i < text.size(); i++) {
		glyph = GetGlyph(text[i], xOff, yOff);
		xOff = glyph.OffsetX;
		yOff = glyph.OffsetY;

		lineHeight = glm::max(lineHeight, -glyph.Positions[1].y);
		maxWidth = glm::max(maxWidth, xOff);

		if (text[i] == '\n')
		{
			yOff += GetLineHeight();
			totalHeight += lineHeight;
			lineHeight = 0.0f;
			xOff = 0;
		} else if (text[i] == '\r') {
			xOff = 0;
		} else if (text[i] == '\t') {
			float xOffTemp{ 0 }, yOffTemp{ 0 };
			glyph = GetGlyph(' ', xOffTemp, yOffTemp);
			xOff += glyph.OffsetX * 4;
		}
	}
	totalHeight += lineHeight;
	return glm::vec2(maxWidth, totalHeight) * scale;
}


GlyphInfo Font::__CreateGlyph(uint32_t index)
{
	stbtt_aligned_quad quad;

	float offsetX{0}, offsetY{0};
	stbtt_GetPackedQuad(_glyphs, _atlasWidth, _atlasHeight, index, &offsetX, &offsetY, &quad, 1);
	float xmin = quad.x0;
	float xmax = quad.x1;
	float ymin = quad.y1;
	float ymax = quad.y0;

	GlyphInfo info = GlyphInfo();

	info.OffsetX      = offsetX;
	info.OffsetY      = offsetY;
	info.Positions[0] = { xmax, ymin };
	info.Positions[1] = { xmax, ymax };
	info.Positions[2] = { xmin, ymax };
	info.Positions[3] = { xmin, ymin };
	info.UVs[0]       = { quad.s1, quad.t1 };
	info.UVs[1]       = { quad.s1, quad.t0 };
	info.UVs[2]       = { quad.s0, quad.t0 };
	info.UVs[3]       = { quad.s0, quad.t1 };
	info.IsPacked = true;

	return info;
}

nlohmann::json Font::ToJson() const
{
	nlohmann::json blob = {
		{ "filename", _fontPath },
		{ "font_size", _fontSize }
	};

	nlohmann::json ranges = std::vector<nlohmann::json>();
	for (auto& range : _glyphRanges) {
		ranges.push_back(range);
	}
	blob["ranges"] = ranges;
	return blob;
}

Font::Sptr Font::FromJson(const nlohmann::json& data) {
	Font::Sptr result = std::make_shared<Font>();
		
	// Load the path and font size so we can grab the font file
	std::string path = JsonGet<std::string>(data, "filename", "");
	float size = JsonGet(data, "font_size", 16.0f);
	result->Load(path, size);
		
	// Iterate over the ranges and add them to the font
	if (data.contains("ranges") && data["ranges"].is_array()) {
		for (const auto& rangeData : data["ranges"]) {
			glm::uvec2 range = rangeData;
			result->AddGlyphRange(range.x, range.y);
		}
	}

	// Bake font texture and return
	result->Bake();
	return result;
}
