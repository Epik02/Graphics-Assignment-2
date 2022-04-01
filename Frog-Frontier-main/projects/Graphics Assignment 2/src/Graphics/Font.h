#pragma once

#include "Utils/ResourceManager/IResource.h"
#include "Graphics/Textures/Texture2D.h"

#include <stb_truetype.h>

	struct GlyphInfo {
		glm::vec2 Positions[4];
		glm::vec2 UVs[4];
		float OffsetX, OffsetY;
		bool IsPacked;
	};

	/// <summary>
	/// The font resource wraps around stb_truetype to allow us to render text to the screen
	/// A Font class contains the texture atlas and data needed to render glyphs using said atlas
	/// </summary>
	class Font : public IResource {
	public:
		typedef std::shared_ptr<Font> Sptr;
		typedef std::weak_ptr<Font> Wptr;


		Font();
		Font(const std::string& fontPath, float size = 16.0f);
		virtual ~Font();

		/// <summary>
		/// Loads a truetype font into this font, replacing the existing font data
		/// if present
		/// </summary>
		/// <param name="fontPath">The path to the truetype font</param>
		/// <param name="size">The size to render to font in the font atlas</param>
		void Load(const std::string& fontPath, float size = 16.0f);

		/// <summary>
		/// Adds a range of unicode characters to enable in this font.
		/// Since we are rendering to a texture, we need to know all of the characters
		/// that we will want to render ahead of time
		/// </summary>
		/// <param name="min">The minimum unicode character (inclusive)</param>
		/// <param name="max">The maximum unicode character (inclusive)</param>
		void AddGlyphRange(uint32_t min, uint32_t max);

		/// <summary>
		/// Generates the texture to use when rendering with this font, must be called
		/// before the font is used
		/// </summary>
		void Bake();
		/// <summary>
		/// Gets the texture atlas for this font
		/// </summary>
		const Texture2D::Sptr& GetAtlas();

		/// <summary>
		/// Extracts information about a glyph with the given codepoint, positioning
		/// it at the offset provided
		/// </summary>
		/// <param name="codePoint">The unicode codepoint to attempt to lookup</param>
		/// <param name="offsetX">The x position of the glyph</param>
		/// <param name="offsetY">The y position of the glyph</param>
		GlyphInfo GetGlyph(uint32_t codePoint, float offsetX, float offsetY) const;
		/// <summary>
		/// Gets the kerning (horizontal space) between 2 unicode characters
		/// </summary>
		/// <param name="char1">The left character</param>
		/// <param name="char2">The right character</param>
		/// <returns>The space between characters</returns>
		float  GetKerning(int char1, int char2) const;
		/// <summary>
		/// Returns the vertical height of a line of text for this font
		/// </summary>
		float  GetLineHeight() const;

		/// <summary>
		/// Measures the size of a string using this font
		/// </summary>
		/// <param name="text">The string to measure</param>
		/// <param name="scale">The scaling to apply to the text, default is 1.0f</param>
		/// <returns>The dimension of the string as rendered with this font</returns>
		virtual glm::vec2 MeausureString(const std::string& text, const float scale = 1.0f);

		/// <summary>
		/// Measures the size of a unicode string using this font
		/// </summary>
		/// <param name="text">The string to measure</param>
		/// <param name="scale">The scaling to apply to the text, default is 1.0f</param>
		/// <returns>The dimension of the string as rendered with this font</returns>
		virtual glm::vec2 MeausureString(const std::wstring& text, const float scale = 1.0f);

		virtual nlohmann::json ToJson() const override;
		static Font::Sptr FromJson(const nlohmann::json& data);

	protected:
		std::vector<glm::uvec2> _glyphRanges;
		std::map<uint32_t, GlyphInfo> _glyphMap;
		GlyphInfo                     _defaultGlyph;
		Texture2D::Sptr   _atlas;
		std::string       _fontPath;
		std::string       _fontData;
		float             _fontSize;

		float             _pixelHeightScale;
		float             _emToPixel;
		int               _ascent,
						  _descent,
						  _lineGap;

		uint32_t          _atlasWidth,
			              _atlasHeight;

		stbtt_packedchar* _glyphs;
		stbtt_fontinfo    _fontInfo;

		GlyphInfo __CreateGlyph(uint32_t index);
	};