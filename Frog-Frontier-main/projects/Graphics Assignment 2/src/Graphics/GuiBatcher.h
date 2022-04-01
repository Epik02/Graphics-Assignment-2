#pragma once

#include <GLM/glm.hpp>

#include "Graphics/Textures/Texture2D.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Utils/MeshBuilder.h"
#include <unordered_map>

	/// <summary>
	/// The GUI Batcher class provides utilities for drawing rectangles and
	/// fonts to the screen in a 2D fashion
	/// </summary>
	class GuiBatcher {
	public:
		/// <summary>
		/// Adds a rectangle to the GUI batch, with a given border radius in pixels.
		/// This can be used with textures to create rounded borders
		/// Creates 9 sub rectangles, note that the center and edge regions will be stretched
		/// </summary>
		/// <param name="min">The minimum bounds in projection space coordinates</param>
		/// <param name="max">The maximum bounds in projection space coordinates</param>
		/// <param name="color">The color multiplier for the image</param>
		/// <param name="tex">The texture to render with</param>
		/// <param name="edgeRadius">The distance in pixels to the edge within the texture for slicing</param>
		static void PushRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& color, const Texture2D::Sptr& tex, int edgeRadius = 0);
		/// <summary>
		/// Adds a rectangle to the GUI batch, with given UV coordinates
		/// </summary>
		/// <param name="min">The minimum bounds in projection space coordinates</param>
		/// <param name="max">The maximum bounds in projection space coordinates</param>
		/// <param name="color">The color multiplier for the image</param>
		/// <param name="tex">The texture to render with</param>
		/// <param name="uvMin">The minimum coord of the UV range</param>
		/// <param name="uvMin">The maximum coord of the UV range</param>
		static void PushRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& color, const Texture2D::Sptr& tex, const glm::vec2 uvMin, const glm::vec2 uvMax);
		/// <summary>
		/// Renders a left-aligned line of text at the given position using a font
		/// </summary>
		/// <param name="text">The unicode text to render</param>
		/// <param name="font">The font to render with</param>
		/// <param name="position">The position of the text in model space</param>
		/// <param name="color">The color of the text</param>
		/// <param name="scale">The scaling to apply to the text</param>
		static void RenderText(const std::wstring& text, const Font::Sptr& font, const glm::vec2& position, const glm::vec4& color, float scale = 1.0f);
		/// <summary>
		/// Renders a left-aligned line of text at the given position using a font
		/// </summary>
		/// <param name="text">The ASCII text to render</param>
		/// <param name="font">The font to render with</param>
		/// <param name="position">The position of the text in model space</param>
		/// <param name="color">The color of the text</param>
		/// <param name="scale">The scaling to apply to the text</param>
		static void RenderText(const std::string& text, const Font::Sptr& font, const glm::vec2& position, const glm::vec4& color, float scale = 1.0f);

		/// <summary>
		/// Sets the projection matrix to use for rendering, should ideally be an orthographic
		/// projection that matches the screen size
		/// </summary>
		static void SetProjection(const glm::mat4& projection);
		/// <summary>
		/// Sets the size of the window in pixels
		/// </summary>
		static void SetWindowSize(const glm::ivec2& size);
		/// <summary>
		/// Draws all geometry to the screen and prepares for the next batch
		/// </summary>
		static void Flush();

		/// <summary>
		/// Push a new transform to the stack, this will be multiplied with the
		/// existing transformation
		/// </summary>
		/// <param name="transform">The local space transform to append</param>
		static void PushModelTransform(const glm::mat3& transform);
		/// <summary>
		/// Pops the last transform off the stack
		/// </summary>
		static void PopModelTransform();

		/// <summary>
		/// Sets a new scissor region in model space. Note that this will invoke a 
		/// flush
		/// </summary>
		/// <param name="min">The minimum bounds of the scissor rectangle</param>
		/// <param name="min">The maximum bounds of the scissor rectangle</param>
		static void PushScissorRect(const glm::vec2& min, const glm::vec2& max);
		/// <summary>
		/// Pops the last scissor region, not that this will invoke a flush
		/// </summary>
		static void PopScissorRect();

		/// <summary>
		/// Sets the default texture to use for the background of GUI objects
		/// </summary>
		static void SetDefaultTexture(const Texture2D::Sptr& value);
		/// <summary>
		/// Returns the current default texture for the background of GUI objects
		/// </summary>
		static const Texture2D::Sptr& GetDefaultTexture();

		/// <summary>
		/// Sets the default border radius of GUI objects in pixels, this should be the
		/// distance from the edge of DefaultTexture to make slices for rounded rectangles
		/// </summary>
		static void SetDefaultBorderRadius(int value);
		/// <summary>
		/// Gets the default border radius in pixels
		/// </summary>
		static int GetDefaultBorderRadius();

	private:
		struct IRect {
			glm::ivec2 Min;
			glm::ivec2 Max;
		};

		struct MeshData {
			MeshBuilder<VertexPosColTex> Builder;
			bool IsFont;
		};

		static glm::ivec2 __windowSize;
		static glm::mat4 __projection;
		static glm::mat3 __model;
		static std::vector<glm::mat3> __modelTransformStack;
		static std::vector<IRect> __scissorRects;
		static ShaderProgram::Sptr __shader;
		static ShaderProgram::Sptr __fontShader;
		static std::unordered_map<Texture2D*, MeshData> _meshBuilders;
		static VertexArrayObject::Sptr __vao;
		static VertexBuffer::Sptr __vbo;
		static IndexBuffer::Sptr __ibo;

		static Texture2D::Sptr __defaultUITexture;
		static int __defaultEdgeRadius;

		static void __StaticInit();
	};
