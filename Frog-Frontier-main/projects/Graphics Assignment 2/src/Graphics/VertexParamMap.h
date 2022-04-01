#pragma once

#include <cstdint>
#include <vector>
#include <GLM/glm.hpp>
#include "Graphics/VertexArrayObject.h"

/// <summary>
/// Structure for mapping and setting a Vertex's attribute based on a vertex declaration
/// </summary>
struct VertexParamMap {
	uint32_t PositionOffset;
	uint32_t NormalOffset;
	uint32_t TextureOffset;
	uint32_t ColorOffset;
	uint32_t ColorSize;
	uint32_t TangentOffset;
	uint32_t BiTangentOffset;

	VertexParamMap() :
		PositionOffset(-1),
		NormalOffset(-1),
		TextureOffset(-1),
		ColorOffset(-1),
		ColorSize(0),
		TangentOffset(-1),
		BiTangentOffset(-1) {}

	VertexParamMap(const std::vector<BufferAttribute>& vDecl) : VertexParamMap() {
		// Loop over all the vertex type's attributes
		for (int ix = 0; ix < vDecl.size(); ix++) {
			// If the attribute is a float3 position, store it's byte offset
			if (vDecl[ix].Usage == AttribUsage::Position && vDecl[ix].Size == 3 && vDecl[ix].Type == AttributeType::Float) {
				PositionOffset = vDecl[ix].Offset;
			}
			// If the attribute is a float3 normal, store it's byte offset
			else if (vDecl[ix].Usage == AttribUsage::Normal && vDecl[ix].Size == 3 && vDecl[ix].Type == AttributeType::Float) {
				NormalOffset = vDecl[ix].Offset;
			}
			// If the attribute is a float2 texture UV, store it's byte offset
			else if (vDecl[ix].Usage == AttribUsage::Texture && vDecl[ix].Size == 2 && vDecl[ix].Type == AttributeType::Float) {
				TextureOffset = vDecl[ix].Offset;
			}
			// If the attribute is a float2 texture UV, store it's byte offset
			else if (vDecl[ix].Usage == AttribUsage::Color && vDecl[ix].Type == AttributeType::Float) {
				ColorOffset = vDecl[ix].Offset;
				ColorSize   = vDecl[ix].Size;
			}
			// If the attribute is a float3 tangent, store it's byte offset
			else if (vDecl[ix].Usage == AttribUsage::Tangent && vDecl[ix].Size == 3 && vDecl[ix].Type == AttributeType::Float) {
				TangentOffset = vDecl[ix].Offset;
			}
			// If the attribute is a float3 bitangent, store it's byte offset
			else if (vDecl[ix].Usage == AttribUsage::BiTangent && vDecl[ix].Size == 3 && vDecl[ix].Type == AttributeType::Float) {
				BiTangentOffset = vDecl[ix].Offset;
			}
		}
	}

	template <typename Vertex>
	void SetPosition(Vertex& vertex, const glm::vec3& value) const {
		if (PositionOffset != (uint32_t)-1) {
			memcpy(GetPtrOffset(vertex, PositionOffset), glm::value_ptr(value), sizeof(glm::vec3));
		}
	}

	template <typename Vertex>
	void SetNormal(Vertex& vertex, const glm::vec3& value) const {
		if (NormalOffset != (uint32_t)-1) {
			memcpy(GetPtrOffset(vertex, NormalOffset), glm::value_ptr(value), sizeof(glm::vec3));
		}
	}

	template <typename Vertex>
	void SetTexture(Vertex& vertex, const glm::vec2& value) const {
		if (TextureOffset != (uint32_t)-1) {
			memcpy(GetPtrOffset(vertex, TextureOffset), glm::value_ptr(value), sizeof(glm::vec2));
		}
	}

	template <typename Vertex>
	void SetColor(Vertex& vertex, const glm::vec4& value) const {
		if (ColorOffset != (uint32_t)-1) {
			memcpy(GetPtrOffset(vertex, ColorOffset), glm::value_ptr(value), sizeof(float) * ColorSize);
		}
	}

	template <typename Vertex>
	void SetTangent(Vertex& vertex, const glm::vec3& value) const {
		if (TangentOffset != (uint32_t)-1) {
			memcpy(GetPtrOffset(vertex, TangentOffset), glm::value_ptr(value), sizeof(glm::vec3));
		}
	}

	template <typename Vertex>
	void SetBiTangent(Vertex& vertex, const glm::vec3& value) const {
		if (BiTangentOffset != (uint32_t)-1) {
			memcpy(GetPtrOffset(vertex, BiTangentOffset), glm::value_ptr(value), sizeof(glm::vec3));
		}
	}


	template <typename Vertex>
	glm::vec3 GetPosition(Vertex& vertex) const {
		if (PositionOffset != (uint32_t)-1) {
			return *GetPtrOffset<Vertex, glm::vec3>(vertex, PositionOffset);
		}
		return glm::vec3(0.0f);
	}

	template <typename Vertex>
	glm::vec3 GetNormal(Vertex& vertex) const {
		if (NormalOffset != (uint32_t)-1) {
			return *GetPtrOffset<Vertex, glm::vec3>(vertex, NormalOffset);
		}
		return glm::vec3(0.0f);
	}

	template <typename Vertex>
	glm::vec2 GetTexture(Vertex& vertex) const {
		if (TextureOffset != (uint32_t)-1) {
			return *GetPtrOffset<Vertex, glm::vec2>(vertex, TextureOffset);
		}
		return glm::vec2(0.0f);
	}

	template <typename Vertex>
	glm::vec4 GetColor(Vertex& vertex) const {
		if (ColorOffset != (uint32_t)-1) {
			switch (ColorSize) {
				case 2:
					return glm::vec4(*GetPtrOffset<Vertex, glm::vec2>(vertex, ColorOffset), 0, 1);
				case 3:
					return glm::vec4(*GetPtrOffset<Vertex, glm::vec3>(vertex, ColorOffset), 1);
				case 4:
					return *GetPtrOffset<Vertex, glm::vec4>(vertex, ColorOffset);
			}
		}
		return glm::vec4(1.0f);
	}

	template <typename Vertex>
	glm::vec3 GetTangent(Vertex& vertex) const {
		if (TangentOffset != (uint32_t)-1) {
			return *GetPtrOffset<Vertex, glm::vec3>(vertex, TangentOffset);
		}
		return glm::vec3(0.0f);
	}

	template <typename Vertex>
	glm::vec3 GetBiTangent(Vertex& vertex) const {
		if (BiTangentOffset != (uint32_t)-1) {
			return *GetPtrOffset<Vertex, glm::vec3>(vertex, BiTangentOffset);
		}
		return glm::vec3(0.0f);
	}

private:
	template <typename Vertex, typename EndType = void>
	EndType* GetPtrOffset(Vertex& vert, uint32_t offset) const {
		return reinterpret_cast<EndType*>(reinterpret_cast<uint8_t*>(&vert) + offset);
	}
};