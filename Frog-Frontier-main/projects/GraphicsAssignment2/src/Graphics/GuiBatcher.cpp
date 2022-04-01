#include "Graphics/GuiBatcher.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "Utils/ResourceManager/ResourceManager.h"
#include <locale>
#include <codecvt>


std::unordered_map<Texture2D*, GuiBatcher::MeshData> GuiBatcher::_meshBuilders;

VertexArrayObject::Sptr GuiBatcher::__vao = nullptr;
IndexBuffer::Sptr GuiBatcher::__ibo = nullptr;

Texture2D::Sptr GuiBatcher::__defaultUITexture = nullptr;
int GuiBatcher::__defaultEdgeRadius = 0;

VertexBuffer::Sptr GuiBatcher::__vbo = nullptr;
ShaderProgram::Sptr GuiBatcher::__shader = nullptr;
ShaderProgram::Sptr GuiBatcher::__fontShader = nullptr;
glm::ivec2 GuiBatcher::__windowSize = {0, 0};
glm::mat4 GuiBatcher::__projection = glm::mat4(1.0f);
glm::mat3 GuiBatcher::__model = glm::mat3(1.0f);
std::vector<glm::mat3> GuiBatcher::__modelTransformStack = std::vector<glm::mat3>();
std::vector<GuiBatcher::IRect> GuiBatcher::__scissorRects = std::vector<GuiBatcher::IRect>();

void GuiBatcher::PushRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& color, const Texture2D::Sptr& tex, const glm::vec2 uvMin, const glm::vec2 uvMax) {
	// Create vertices and transform positions
	VertexPosColTex verts[4];
	verts[0].Position = __model * glm::vec3(min.x, min.y, 1.0f);
	verts[1].Position = __model * glm::vec3(min.x, max.y, 1.0f);
	verts[2].Position = __model * glm::vec3(max.x, max.y, 1.0f);
	verts[3].Position = __model * glm::vec3(max.x, min.y, 1.0f);
		
	// Grab mesh info for the texture batch
	MeshData& mesh = _meshBuilders[tex.get()];
	// We can use the vertex count for depth, so that things drawn later have a bit of spacing
	float depth = mesh.Builder.GetVertexCount() / 1000.0f;

	// Copy in all color and set depth 
	for (int ix = 0; ix < 4; ix++) {
		verts[ix].Color = color;
		verts[ix].Position.z = depth;
	}

	// Copy over UV coords
	verts[0].UV = glm::vec2(uvMin.x, uvMax.y);
	verts[1].UV = glm::vec2(uvMin.x, uvMin.y);
	verts[2].UV = glm::vec2(uvMax.x, uvMin.y);
	verts[3].UV = glm::vec2(uvMax.x, uvMax.y);

	// Add vertices and indices to range
	uint32_t ix = mesh.Builder.AddVertexRange(verts, 4);
	mesh.Builder.AddIndexTri(ix + 0, ix + 2, ix + 1);
	mesh.Builder.AddIndexTri(ix + 0, ix + 3, ix + 2);
}

void GuiBatcher::PushRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& color, const Texture2D::Sptr& tex, int edgeRadius)
{
	if (edgeRadius <= 0) {
		PushRect(min, max, color, tex, { 0,0 }, { 1,1 });
	} 
	else {
		glm::vec2 edgeOffset;
		edgeOffset.x = edgeRadius / ((float)tex->GetWidth() - 2);
		edgeOffset.y = edgeRadius / ((float)tex->GetHeight() - 2);
		
		// edge [min/max] [X,Y] screen
		float eMinXS = min.x + edgeRadius;
		float eMaxXS = max.x - edgeRadius;
		float eMinYS = min.y + edgeRadius;
		float eMaxYS = max.y - edgeRadius;

		// UV [min/max] [X,Y]
		float uMinXS = edgeOffset.x;
		float uMaxXS = 1.0f - edgeOffset.x;
		float uMinYS = edgeOffset.y;
		float uMaxYS = 1.0f - edgeOffset.y;

		// Left column
		PushRect(glm::vec2(min.x, min.y),  glm::vec2(eMinXS, eMinYS), color, tex, glm::vec2(0.0f, uMaxYS),   glm::vec2(uMinXS, 1.0f));
		PushRect(glm::vec2(min.x, eMinYS), glm::vec2(eMinXS, eMaxYS), color, tex, glm::vec2(0.0f, uMinYS), glm::vec2(uMinXS, uMaxYS));
		PushRect(glm::vec2(min.x, eMaxYS), glm::vec2(eMinXS, max.y),  color, tex, glm::vec2(0.0f, 0.0f), glm::vec2(uMinXS, uMinYS));

		// Center column
		PushRect(glm::vec2(eMinXS, min.y),  glm::vec2(eMaxXS, eMinYS), color, tex, glm::vec2(uMinXS, uMaxYS),   glm::vec2(uMaxXS, 1.0f));
		PushRect(glm::vec2(eMinXS, eMinYS), glm::vec2(eMaxXS, eMaxYS), color, tex, glm::vec2(uMinXS, uMinYS), glm::vec2(uMaxXS, uMaxYS));
		PushRect(glm::vec2(eMinXS, eMaxYS), glm::vec2(eMaxXS, max.y), color, tex,  glm::vec2(uMinXS, 0.0f), glm::vec2(uMaxXS, uMinYS));

		// Right column
		PushRect(glm::vec2(eMaxXS, min.y),  glm::vec2(max.x, eMinYS), color, tex, glm::vec2(uMaxXS, uMaxYS),   glm::vec2(1.0f, 1.0f));
		PushRect(glm::vec2(eMaxXS, eMinYS), glm::vec2(max.x, eMaxYS), color, tex, glm::vec2(uMaxXS, uMinYS), glm::vec2(1.0f, uMaxYS));
		PushRect(glm::vec2(eMaxXS, eMaxYS), glm::vec2(max.x, max.y),  color, tex, glm::vec2(uMaxXS, 0.0f), glm::vec2(1.0f, uMinYS));
	}
}

void GuiBatcher::SetProjection(const glm::mat4& projection) {
	__projection = projection;
}

void GuiBatcher::RenderText(const std::wstring& text, const Font::Sptr& font, const glm::vec2& position, const glm::vec4& color, float scale /*= 1.0f*/) {
	// How many characters we have
	size_t length = text.size();

	// Tracks the offset of the character
	glm::vec2 offset = glm::vec2(0.0f);

	// Transform the origin based off the model transform
	glm::vec2 origin = position;

	// Gets the texture used to render the font
	Texture2D::Sptr atlas = font->GetAtlas();

	// Grab the mesh builder and make sure it's a texture batch
	MeshData& mesh = _meshBuilders[atlas.get()];
	mesh.IsFont = true;

	// Allocate some space for the vertices
	VertexPosColTex verts[4];
	verts[0].Color = color;
	verts[1].Color = color;
	verts[2].Color = color;
	verts[3].Color = color;

	float depth = mesh.Builder.GetVertexCount() / 1000.0f;

	// Iterate over all characters in string
	for (int i = 0; i < length; i++) {
		// Grab the glyph data for the character
		GlyphInfo glyph = font->GetGlyph(text[i], offset.x, offset.y);

		// A newline will advance to the next line and return to the start of the line
		if (text[i] == '\n') {
			offset.y  += font->GetLineHeight() * scale;
			offset.x = 0;
		}
		// A return character simply returns to the start of the line
		else if (text[i] == '\r') {
			offset.x = 0;
		}
		// A tab character is 4 spaces
		else if (text[i] == '\t') {
			float xOffTemp{ 0 }, yOffTemp{ 0 };
			glyph = font->GetGlyph(' ', xOffTemp, yOffTemp);
			offset.x += glyph.OffsetX * 4;
		}
		// All other characters get rendered
		else {
			verts[0].Position = __model * glm::vec3(origin + (offset + glyph.Positions[0]) * scale, 1.0f);
			verts[1].Position = __model * glm::vec3(origin + (offset + glyph.Positions[1]) * scale, 1.0f);
			verts[2].Position = __model * glm::vec3(origin + (offset + glyph.Positions[2]) * scale, 1.0f);
			verts[3].Position = __model * glm::vec3(origin + (offset + glyph.Positions[3]) * scale, 1.0f);
			verts[0].UV = glyph.UVs[0];
			verts[1].UV = glyph.UVs[1];
			verts[2].UV = glyph.UVs[2];
			verts[3].UV = glyph.UVs[3];

			verts[0].Position.z = verts[1].Position.z = verts[2].Position.z = verts[3].Position.z = depth;

			uint32_t ix = mesh.Builder.AddVertexRange(verts, 4);
			mesh.Builder.AddIndexTri(ix + 0, ix + 1, ix + 2);
			mesh.Builder.AddIndexTri(ix + 0, ix + 2, ix + 3);

			// Advance the offset based on the size of the glyph
			offset.x = glyph.OffsetX;
			offset.y = glyph.OffsetY;

			// If we have more characters, see if there's any kerning between the
			// current and next character and add it to the x offset
			if (i < length - 1) {
				float kerning = font->GetKerning(text[i], text[i + 1]);
				offset.x += kerning;
			}
		}

	}

}

void GuiBatcher::RenderText(const std::string& text, const Font::Sptr& font, const glm::vec2& position, const glm::vec4& color, float scale /*= 1.0f*/)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	RenderText(converter.from_bytes(text), font, position, color, scale);
}

void GuiBatcher::Flush()
{
	__StaticInit();

	// Iterate over each texture and it's mesh
	for (auto&[key, value] : _meshBuilders) {
		Texture2D* tex = key;
		// If the texture exists and the mesh has data
		if (tex != nullptr && value.Builder.GetIndexCount() > 0) {
			// Update the VAO and it's buffers
			__vao->Bind();
			__vbo->UpdateData(value.Builder.GetVertexDataPtr(), sizeof(VertexPosColTex), value.Builder.GetVertexCount(), true);
			__ibo->UpdateData(value.Builder.GetIndexDataPtr(), sizeof(uint32_t), value.Builder.GetIndexCount(), true);

			// Bind texture, send uniforms to shader
			tex->Bind(0);
			ShaderProgram::Sptr shader = value.IsFont ? __fontShader : __shader;
			shader->Bind();
			shader->SetUniformMatrix(0, &__projection, 1, false);

			// Draw geometry
			__vao->Draw();

			// Clear mesh
			value.Builder.Reset();
		}
	}
}

void GuiBatcher::PushModelTransform(const glm::mat3& transform) {
	__modelTransformStack.push_back(transform);
	__model = __model * transform;
}

void GuiBatcher::PopModelTransform()
{
	LOG_ASSERT(__modelTransformStack.size() > 0, "Transform push/pop mismatch");
	__model = __model * glm::inverse(__modelTransformStack.back());
	__modelTransformStack.pop_back();
}

void GuiBatcher::SetWindowSize(const glm::ivec2& size) {
	__windowSize = size;
}

void GuiBatcher::__StaticInit()
{
	static bool needsInit = true;
	if (needsInit) {
		__shader = ShaderProgram::Create();
		__shader->LoadShaderPart(R"LIT(#version 460
					layout(location = 0) in vec3 inPos;
					layout(location = 1) in vec4 inColor;
					layout(location = 3) in vec2 inUV;

					layout(location = 0) out vec4 outColor;
					layout(location = 1) out vec2 outUV;

					layout(location = 0) uniform mat4 u_Projection;

					void main() {
						outColor = inColor;
						outUV = inUV;
						gl_Position = u_Projection * vec4(inPos, 1);
					}
				)LIT", ShaderPartType::Vertex);

		__shader->LoadShaderPart(R"LIT(#version 460
					layout(location = 0) in vec4 inColor;
					layout(location = 1) in vec2 inUV;

					layout(location = 0) out vec4 outColor;

					uniform layout(binding=0) sampler2D s_Texture;

					void main() {
						outColor = texture(s_Texture, inUV) * inColor;
					}
				)LIT", ShaderPartType::Fragment);

		__shader->Link();

		__fontShader = ShaderProgram::Create();
		__fontShader->LoadShaderPart(R"LIT(#version 460
					layout(location = 0) in vec3 inPos;
					layout(location = 1) in vec4 inColor;
					layout(location = 3) in vec2 inUV;

					layout(location = 0) out vec4 outColor;
					layout(location = 1) out vec2 outUV;

					layout(location = 0) uniform mat4 u_Projection;

					void main() {
						outColor = inColor;
						outUV = inUV;
						gl_Position = u_Projection * vec4(inPos, 1);
					}
				)LIT", ShaderPartType::Vertex);

		__fontShader->LoadShaderPart(R"LIT(#version 460
					layout(location = 0) in vec4 inColor;
					layout(location = 1) in vec2 inUV;

					layout(location = 0) out vec4 outColor;

					uniform layout(binding=0) sampler2D s_Texture;

					void main() {
						float fontPow = texture(s_Texture, inUV).r;
						outColor = vec4(inColor.rgb, fontPow);
					}
				)LIT" , ShaderPartType::Fragment);

		__fontShader->Link();

		__vbo = VertexBuffer::Create(BufferUsage::DynamicDraw);
		__ibo = IndexBuffer::Create(BufferUsage::DynamicDraw, IndexType::UInt);

		__vao = VertexArrayObject::Create();
		__vao->AddVertexBuffer(__vbo, VertexPosColTex::V_DECL);
		__vao->SetIndexBuffer(__ibo);

		// Generate a simple white texture with a black border
		if (__defaultUITexture == nullptr) {
			Texture2DDescription desc = Texture2DDescription();
			desc.Width = 16;
			desc.Height = 16;
			desc.MinificationFilter = MinFilter::Nearest;
			desc.MagnificationFilter = MagFilter::Nearest;
			desc.Format = InternalFormat::RGBA8;

			__defaultUITexture = ResourceManager::CreateAsset<Texture2D>(desc);
			glm::u8vec4 data[16 * 16];
			// Set everything to white by default
			memset(data, 255, 16 * 16 * 4);
			// Generate a black border around the image
			for (int ix = 0; ix < 16; ix++) {
				for (int iy = 0; iy < 16; iy++) {
					if (ix == 0 || iy == 0 || ix == 15 || iy == 15) {
						data[ix * 16 + iy] ={ 0,0,0, 255 };
					}
				}
			}
			__defaultUITexture->LoadData(16, 16, PixelFormat::RGBA, PixelType::UByte, data);
		}

		needsInit = false;
	}
}

void GuiBatcher::PushScissorRect(const glm::vec2& min, const glm::vec2& max) {
	// Convert input to the current space
	glm::vec2 modelMin = __model * glm::vec3(min, 1.0f);
	glm::vec2 modelMax = __model * glm::vec3(max, 1.0f);

	// Project the values, will be in Normalized Device Coordinates ([-1,1])
	glm::vec2 minNDC = __projection * glm::vec4(modelMin, 0.0f, 1.0f);
	glm::vec2 maxNDC = __projection * glm::vec4(modelMax, 0.0f, 1.0f);

	// Convert NDC to screenspace
	glm::ivec2 minWin = glm::floor(((minNDC + 1.0f) / 2.0f) * (glm::vec2)__windowSize);
	glm::ivec2 maxWin = glm::ceil(((maxNDC + 1.0f) / 2.0f)  * (glm::vec2)__windowSize);

	// Store the bounds
	__scissorRects.push_back({ minWin, maxWin });

	// Calculate the actual size
	int width  = glm::max(maxWin.x, minWin.x) - glm::min(maxWin.x, minWin.x);
	int height = glm::max(maxWin.y, minWin.y) - glm::min(maxWin.y, minWin.y);

	// Draw current geo with the current scissor, then update it
	Flush();
	glScissor(minWin.x, maxWin.y, width, height);
}

void GuiBatcher::PopScissorRect() {
	LOG_ASSERT(__scissorRects.size() > 0, "Scissor rect push/pop mismatch!");
	__scissorRects.pop_back();

	// Grab the last scissor rect, or just use window bounds if none left
	IRect bounds = __scissorRects.size() > 0 ? __scissorRects.back() : IRect{ { 0, 0 }, { __windowSize.x, __windowSize.y } };

	// Calculate the actual size
	int width  = glm::max(bounds.Min.x, bounds.Max.x) - glm::min(bounds.Min.x, bounds.Max.x);
	int height = glm::max(bounds.Min.y, bounds.Max.y) - glm::min(bounds.Min.y, bounds.Max.y);

	// Draw current geo with the current scissor, then update it
	Flush();
	glScissor(glm::min(bounds.Min.x, bounds.Max.x), glm::min(bounds.Min.y, bounds.Max.y), width, height);
}

void GuiBatcher::SetDefaultTexture(const Texture2D::Sptr& value) {
	__defaultUITexture = value;
}

const Texture2D::Sptr& GuiBatcher::GetDefaultTexture() {
	return __defaultUITexture;
}

void GuiBatcher::SetDefaultBorderRadius(int value) {
	__defaultEdgeRadius = value;
}

int GuiBatcher::GetDefaultBorderRadius() {
	return __defaultEdgeRadius;
}
