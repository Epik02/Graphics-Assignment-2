#pragma once
// NOTE: We use a .inl file here so we can define an implementation for the templates outside of the header file, keeping it much cleaner

#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/euler_angles.hpp>
#include <unordered_map>
#include <GLM/gtc/type_ptr.hpp> // for glm::value_ptr
#include "Graphics/VertexArrayObject.h"
#include "Logging.h"
#include "MeshFactory.h"
#include "Utils/JsonGlmHelpers.h"
#include "Graphics/VertexParamMap.h"

#define M_PI 3.14159265359f

struct RGBA_Helper {
	union {
		struct {
			uint8_t R;
			uint8_t G;
			uint8_t B;
			uint8_t A;
		};
		uint32_t HexCode;
	};
};

/// <summary>
/// Helper function for creating a vertx and setting all fields if they exist
/// </summary>
/// <typeparam name="Vertex">The type of vertex to create</typeparam>
/// <param name="pos">The position for the new vertex</param>
/// <param name="norm">The normal for the new vertex</param>
/// <param name="uv">The UV coordinates for the new vertex</param>
/// <param name="col">The color for the new vertex</param>
/// <param name="vMap">The mapping of vertex attributes to use</param>
/// <returns>A new vertex with the given properties</returns>
template <typename Vertex>
Vertex Create(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv, glm::vec4 col, const VertexParamMap& vMap) {
	Vertex result;
	vMap.SetPosition(result, pos);
	vMap.SetNormal(result, norm);
	vMap.SetTexture(result, uv);
	vMap.SetColor(result, col);
	return result;
}

/// <summary>
/// Adds a mid point between two points on an ico sphere to the list of vertices, or optionally gets it from a cache of existing midpoints
/// </summary>
/// <typeparam name="Vertex">The type of vertex of the mesh to operate on</typeparam>
/// <param name="scale">The radius scaling of the sphere</param>
/// <param name="center">The center of the sphere</param>
/// <param name="a">The index of the first vertex</param>
/// <param name="b">The index of the second vertex</param>
/// <param name="vertices">The collection of vertices to work in</param>
/// <param name="midpointCache">A map to use for caching existing vertices</param>
/// <param name="vMap">The mapping of vertex attributes to use</param>
/// <returns>The index of the vertex in the vertices list</returns>
template <typename Vertex>
uint32_t AddMiddlePoint(glm::vec3 scale, glm::vec3 center, uint32_t a, uint32_t b, std::vector<Vertex>& vertices, std::unordered_map<uint64_t, uint32_t>& midpointCache, const VertexParamMap& vMap)
{
	// We calculate a unique 64 bit key for each index combination (order independent)
	uint64_t key = 0;
	if (a < b) {
		key = (static_cast<uint64_t>(a) << 32ul) | static_cast<uint64_t>(b);
	} else {
		key = (static_cast<uint64_t>(b) << 32ul) | static_cast<uint64_t>(a);
	}
	// Attempt to find the midpoint between those 2 vertices
	auto it = midpointCache.find(key);

	// We had a midpoint already, return it's index
	if (it != midpointCache.end())
		return it->second;
	// We do not have a midpoint, we need to calculate and add a new one
	else {
		// Get the two vertices that we are interpolating
		Vertex p1 = vertices[a];
		Vertex p2 = vertices[b];

		// Create a new vertex
		Vertex interpolated;

		// Calculate position based on the average between the two normals and store
		glm::vec3 p1Norm = glm::normalize(vMap.GetPosition(p1) - center);
		glm::vec3 p2Norm = glm::normalize(vMap.GetPosition(p2) - center);
		glm::vec3 pos = glm::normalize((p1Norm + p2Norm) / 2.0f);
		vMap.SetPosition(interpolated, center + (pos * scale));

		// The normal is just the position
		vMap.SetNormal(interpolated, pos);
		
		// Calculate the UV coordinates for the point and store them
		float u = atan2f(pos.y, pos.x) / (2.0f * M_PI);
		float v = (asinf(pos.z) / M_PI) + 0.5f;
		vMap.SetTexture(interpolated, glm::vec2(u, v));

		// If the vertex has a color, interpolate the color and store it
		if (vMap.ColorOffset != -1) {
			vMap.SetColor(interpolated, (vMap.GetColor(p1) + vMap.GetColor(p2)) / 2.0f);
		}

		// Get the index of the new vertex, store it in the cache
		uint32_t ix = static_cast<uint32_t>(vertices.size());
		midpointCache[key] = ix;

		// Add vertex to data and return it's index
		vertices.push_back(interpolated);
		return ix;
	}
}

/// <summary>
/// Handles correcting UV seams on spheres
/// </summary>
/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
/// <param name="verts">The vertices to work on</param>
/// <param name="indices">The indices of the triangles in the mesh</param>
/// <param name="offset">The offset into the index buffer to start modifying from, should be a multiple of 3</param>
/// <param name="vMap">The mapping of vertex attributes to use</param>
template <typename Vertex>
void CorrectUVSeams(std::vector<Vertex>& verts, std::vector<uint32_t>& indices, size_t offset, const VertexParamMap& vMap) {
	// Early bail if the vertex type does not have a texture attribute
	if (vMap.TextureOffset == -1) return;

	// lambda closure to easily add a vertex with unique texture coordinate to our mesh
	auto addVertex = [&](size_t ix, const glm::vec2& uv) {
		const uint32_t index = indices[ix];
		indices[ix] = (uint32_t)verts.size();
		Vertex vert = verts[index];
		vMap.SetTexture(vert, uv);
		verts.push_back(vert);
	};

	// fix texture seams (this is where the magic happens) TODO: fix seams on tops and bottoms
	const size_t current_tris = offset / 3;
	const size_t numTriangles = (indices.size() - offset) / 3;
	// Iterate over all the triangles we added 
	for (size_t i = current_tris; i < current_tris + numTriangles; ++i) {
		// Get the UV coordinates
		const glm::vec2& uv0 = vMap.GetTexture(verts[indices[i * 3 + 0]]);
		const glm::vec2& uv1 = vMap.GetTexture(verts[indices[i * 3 + 1]]);
		const glm::vec2& uv2 = vMap.GetTexture(verts[indices[i * 3 + 2]]);

		const float d1 = uv1.x - uv0.x;
		const float d2 = uv2.x - uv0.x;

		if (abs(d1) > 0.5f && abs(d2) > 0.5f)
			addVertex(i * 3 + 0, uv0 + glm::vec2((d1 > 0.0f) ? 1.0f : -1.0f, 0.0f));
		else if (abs(d1) > 0.5f)
			addVertex(i * 3 + 1, uv1 + glm::vec2((d1 < 0.0f) ? 1.0f : -1.0f, 0.0f));
		else if (abs(d2) > 0.5f)
			addVertex(i * 3 + 2, uv2 + glm::vec2((d2 < 0.0f) ? 1.0f : -1.0f, 0.0f));
	}
}

/// <summary>
/// Calculates the position, normal, and UV coords for a vertex on a sphere (used for iso-sphere generation)
/// </summary>
/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
/// <param name="norm">The normal of the vertex to calculate</param>
/// <param name="scale">The scale of the sphere</param>
/// <param name="center">The center point of the sphere</param>
/// <param name="color">The color of the sphere vertex</param>
/// <param name="vMap">The mapping of vertex attributes to use</param>
/// <returns>A vertex with the position, normal, and UV components set</returns>
template <typename Vertex>
Vertex CalculateSphereVert(glm::vec3 norm, glm::vec3 scale, glm::vec3 center, const VertexParamMap& vMap, glm::vec4 color = glm::vec4(1.0f)) {
	Vertex vert;
	glm::vec3 pos = glm::normalize(norm);
	float u = atan2f(pos.y, pos.x) / (2.0f * M_PI);
	float v = (asinf(pos.z) / M_PI) + 0.5f;
	vMap.SetPosition(vert, center + (pos * scale));
	vMap.SetNormal(vert, pos);
	vMap.SetTexture(vert, glm::vec2(u, v));
	vMap.SetColor(vert, color);
	return vert;
}

template <typename Vertex>
void MeshFactory::AddIcoSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, float radius, int tessellation, const glm::vec4& col) {
	AddIcoSphere<Vertex>(data, center, glm::vec3(radius), tessellation, col);
}

template <typename Vertex>
void MeshFactory::AddIcoSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, const glm::vec3& radii, int tessellation, const glm::vec4& col) {
	LOG_ASSERT(tessellation >= 0, "Tessellation must be greater than zero!");

	VertexParamMap vMap = VertexParamMap(Vertex::V_DECL);
	if (vMap.PositionOffset == -1) {
		LOG_WARN("Vertex type does not have position attribute, aborting AddIcoSphere");
		return;
	}

	uint32_t indexOffset  = static_cast<uint32_t>(data.GetVertexCount());
	uint32_t initialIndex = static_cast<uint32_t>(data.GetIndexCount());
	std::vector<glm::ivec3> faces;

	uint32_t estimatedFaces = 12 + pow(tessellation + 1, 3);

	float t = (1.0f + sqrtf(5.0f)) / 2.0f;

	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(-1, t, 0),  radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(1, t, 0),   radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(-1, -t, 0), radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(1, -t, 0),  radii, center, vMap, col));

	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(0, -1, t),  radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(0, 1, t),   radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(0, -1, -t), radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(0, 1, -t),  radii, center, vMap, col));

	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(t, 0, -1),  radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(t, 0, 1),   radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(-t, 0, -1), radii, center, vMap, col));
	data._vertices.emplace_back(CalculateSphereVert<Vertex>(glm::vec3(-t, 0, 1),  radii, center, vMap, col));

	glm::ivec3 iOff = glm::ivec3(indexOffset);

	// 5 faces around point 0
	faces.emplace_back(iOff + glm::ivec3(0, 11, 5));
	faces.emplace_back(iOff + glm::ivec3(0, 5, 1));
	faces.emplace_back(iOff + glm::ivec3(0, 1, 7));
	faces.emplace_back(iOff + glm::ivec3(0, 7, 10));
	faces.emplace_back(iOff + glm::ivec3(0, 10, 11));

	// 5 adjacent faces
	faces.emplace_back(iOff + glm::ivec3(1, 5, 9));
	faces.emplace_back(iOff + glm::ivec3(5, 11, 4));
	faces.emplace_back(iOff + glm::ivec3(11, 10, 2));
	faces.emplace_back(iOff + glm::ivec3(10, 7, 6));
	faces.emplace_back(iOff + glm::ivec3(7, 1, 8));

	// 5 faces around point 3
	faces.emplace_back(iOff + glm::ivec3(3, 9, 4));
	faces.emplace_back(iOff + glm::ivec3(3, 4, 2));
	faces.emplace_back(iOff + glm::ivec3(3, 2, 6));
	faces.emplace_back(iOff + glm::ivec3(3, 6, 8));
	faces.emplace_back(iOff + glm::ivec3(3, 8, 9));

	// 5 adjacent faces
	faces.emplace_back(iOff + glm::ivec3(4, 9, 5));
	faces.emplace_back(iOff + glm::ivec3(2, 4, 11));
	faces.emplace_back(iOff + glm::ivec3(6, 2, 10));
	faces.emplace_back(iOff + glm::ivec3(8, 6, 7));
	faces.emplace_back(iOff + glm::ivec3(9, 8, 1));

	// Cache used to index our midpoints
	std::unordered_map<uint64_t, uint32_t> midPointCache;

	for (int ix = 0; ix < tessellation; ix++)
	{
		std::vector<glm::ivec3> tempFaces;
		for (auto& indices : faces)
		{
			uint32_t a = AddMiddlePoint(radii, center, indices[0], indices[1], data._vertices, midPointCache, vMap);
			uint32_t b = AddMiddlePoint(radii, center, indices[1], indices[2], data._vertices, midPointCache, vMap);
			uint32_t c = AddMiddlePoint(radii, center, indices[2], indices[0], data._vertices, midPointCache, vMap);

			tempFaces.emplace_back(glm::ivec3(indices[0], a, c));
			tempFaces.emplace_back(glm::ivec3(indices[1], b, a));
			tempFaces.emplace_back(glm::ivec3(indices[2], c, b));
			tempFaces.emplace_back(glm::ivec3(a, b, c));
		}
		faces = tempFaces;
	}

	for (auto& face : faces) {
		data._indices.push_back(face[0]);
		data._indices.push_back(face[1]);
		data._indices.push_back(face[2]);
	}

	CorrectUVSeams(data._vertices, data._indices, initialIndex, vMap);

}

template <typename Vertex>
void MeshFactory::AddUvSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, float radius, int tessellation, const glm::vec4& col) {
	AddUvSphere(data, center, glm::vec3(radius), tessellation, col);
}

template <typename Vertex>
void MeshFactory::AddUvSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, const glm::vec3& radii, int tessellation, const glm::vec4& col) {
	LOG_ASSERT(tessellation >= 0, "Tessellation must be greater than zero!");

	VertexParamMap vMap = VertexParamMap(Vertex::V_DECL);
	if (vMap.PositionOffset == -1) {
		LOG_WARN("Vertex type does not have position attribute, aborting AddIcoSphere");
		return;
	}

	int slices = 1 + pow(2, tessellation + 1);
	int stacks = (slices / 2) + 1;

	int numverts = (slices + 1) * (slices - 1) + 2;
	std::vector<Vertex>& verts = data._vertices;

	uint32_t offset       = static_cast<uint32_t>(verts.size());
	uint32_t initialIndex = static_cast<uint32_t>(data._indices.size());
	verts.reserve(verts.size() + numverts);

	float stackAngle, sliceAngle;
	float x, y, z, xy;

	float dLong = (M_PI * 2) / slices;
	float dLat = M_PI / stacks;

	for (int i = 0; i <= stacks; ++i) {
		stackAngle = M_PI / 2.0f - i * dLat;
		xy = cosf(stackAngle);
		z = sinf(stackAngle);

		for (int j = 0; j <= slices; ++j) {
			sliceAngle = j * dLong;

			x = xy * cosf(sliceAngle);
			y = xy * sinf(sliceAngle);

			glm::vec3 normal = glm::vec3(x, y, z);
			glm::vec2 uv = glm::vec2((float)j / slices, 1.0f - (float)i / stacks);

			Vertex vert;
			vMap.SetNormal(vert, normal);
			vMap.SetPosition(vert, center + (normal * radii));
			vMap.SetTexture(vert, uv);
			vMap.SetColor(vert, col);
			verts.push_back(vert);
		}
	}
	vMap.SetTexture(verts[offset], { 0.5f, 1.0f });
	vMap.SetTexture(verts[verts.size() - 1], { 0.5f, 0.0f });
		
	int numIndices = (slices - 1) * slices * 6;
	data._indices.reserve(data._indices.size() + numIndices);

	// Body loop
	int k1, k2;
	for (int i = 0; i < stacks; ++i)
	{
		k1 = i * (slices + 1);
		k2 = k1 + slices + 1;
		for (int j = 0; j < slices; ++j, ++k1, ++k2)
		{
			// Our top loop
			if (i != 0) {
				data._indices.push_back(offset + k1);
				data._indices.push_back(offset + k2);
				data._indices.push_back(offset + k1 + 1);
			}

			// Everything but our bottom loop
			if (i != (stacks - 1)) {
				data._indices.push_back(offset + k1 + 1);
				data._indices.push_back(offset + k2);
				data._indices.push_back(offset + k2 + 1);
			}
		}
	}
}

template <typename Vertex>
void MeshFactory::AddPlane(MeshBuilder<Vertex>& mesh, const glm::vec3& pos, const glm::vec3& normal,
	const glm::vec3& tangent, const glm::vec2& scale, const glm::vec2& uvScale, const glm::vec4& col)
{
	VertexParamMap vMap = VertexParamMap(Vertex::V_DECL);
	if (vMap.PositionOffset == -1) {
		LOG_WARN("Vertex type does not have position attribute, aborting AddIcoSphere");
		return;
	}

	glm::vec3 nNorm = glm::normalize(normal);
	glm::vec3 nTangent = glm::normalize(tangent);
	glm::vec3 binormal = glm::cross(nNorm, nTangent);
	glm::vec2 hScale = scale / 2.0f;
	glm::vec3 positions[4] = {
		pos - (nTangent * hScale.x) - (binormal * hScale.y),
		pos - (nTangent * hScale.x) + (binormal * hScale.y),
		pos + (nTangent * hScale.x) + (binormal * hScale.y),
		pos + (nTangent * hScale.x) - (binormal * hScale.y),
	};

	glm::vec2 uvs[] = {
		glm::vec2(0.0f, 0.0f) * uvScale, // 0
		glm::vec2(0.0f, 1.0f) * uvScale, // 1
		glm::vec2(1.0f, 1.0f) * uvScale, // 2
		glm::vec2(1.0f, 0.0f) * uvScale, // 3
	};

	Vertex verts[4];
	for(int ix = 0; ix < 4; ix++) {
		vMap.SetPosition(verts[ix], positions[ix]);
		vMap.SetNormal(verts[ix], nNorm);
		vMap.SetTexture(verts[ix], uvs[ix]);
		vMap.SetColor(verts[ix], col);
	}


	const uint32_t p1 = mesh.AddVertex(positions[0], nNorm, uvs[0], col);
	const uint32_t p2 = mesh.AddVertex(positions[1], nNorm, uvs[1], col);
	const uint32_t p3 = mesh.AddVertex(positions[2], nNorm, uvs[2], col);
	const uint32_t p4 = mesh.AddVertex(positions[3], nNorm, uvs[3], col);

	mesh.AddIndexTri(p1, p3, p2);
	mesh.AddIndexTri(p1, p4, p3);
}

template <typename Vertex>
void MeshFactory::AddCube(MeshBuilder<Vertex>& mesh, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& eulerDeg, const glm::vec4& col) {
	const glm::mat4 transform = glm::translate(MAT4_IDENTITY, pos) * glm::mat4(glm::quat(glm::radians(eulerDeg))) * glm::scale(MAT4_IDENTITY, scale);
	AddCube(mesh, transform, col);
}

template <typename Vertex>
void MeshFactory::AddCube(MeshBuilder<Vertex>& mesh, const glm::mat4& transform, const glm::vec4& col) {

	VertexParamMap vMap = VertexParamMap(Vertex::V_DECL);
	if (vMap.PositionOffset == -1) {
		LOG_WARN("Vertex type does not have position attribute, aborting AddIcoSphere");
		return;
	}

	glm::vec3 positions[] = {
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f,  0.5f, -0.5f),
		glm::vec3(0.5f,  0.5f, -0.5f),

		glm::vec3(-0.5f, -0.5f,  0.5f),
		glm::vec3(0.5f, -0.5f,  0.5f),
		glm::vec3(-0.5f,  0.5f,  0.5f),
		glm::vec3(0.5f,  0.5f,  0.5f),
	};
	for (auto& position : positions) {
		position = transform * glm::vec4(position, 1.0f);
	}

	glm::vec3 normals[] = {
		glm::vec3(-1.0f,  0.0f,  0.0f), //0
		glm::vec3(1.0f,  0.0f,  0.0f),
		glm::vec3(0.0f, -1.0f,  0.0f), //2
		glm::vec3(0.0f,  1.0f,  0.0f),
		glm::vec3(0.0f,  0.0f, -1.0f), //4
		glm::vec3(0.0f,  0.0f,  1.0f),
	};
	glm::mat3 rotation = transform; // Truncate the transform to just the rotation component for normals
	for (auto& normal : normals) {
		normal = rotation * normal;
	}

	glm::vec2 uvs[] = {
		glm::vec2(0.0f, 0.0f), // 0
		glm::vec2(0.0f, 1.0f), // 1
		glm::vec2(1.0f, 1.0f), // 2
		glm::vec2(1.0f, 0.0f), // 3
	};

	uint32_t indices[36];

	#pragma region AddVerts

	mesh.ReserveVertexSpace(24);

	// Bottom
	indices[0] = mesh.AddVertex(Create<Vertex>(positions[0], normals[4], uvs[0], col, vMap));
	indices[1] = mesh.AddVertex(Create<Vertex>(positions[2], normals[4], uvs[1], col, vMap));
	indices[2] = mesh.AddVertex(Create<Vertex>(positions[3], normals[4], uvs[2], col, vMap));
	indices[3] = mesh.AddVertex(Create<Vertex>(positions[1], normals[4], uvs[3], col, vMap));
	// Top
	indices[4] = mesh.AddVertex(Create<Vertex>(positions[6], normals[5], uvs[0], col, vMap));
	indices[5] = mesh.AddVertex(Create<Vertex>(positions[4], normals[5], uvs[1], col, vMap));
	indices[6] = mesh.AddVertex(Create<Vertex>(positions[5], normals[5], uvs[2], col, vMap));
	indices[7] = mesh.AddVertex(Create<Vertex>(positions[7], normals[5], uvs[3], col, vMap));

	// Left
	indices[8]  = mesh.AddVertex(Create<Vertex>(positions[0], normals[0], uvs[0], col, vMap));
	indices[9]  = mesh.AddVertex(Create<Vertex>(positions[4], normals[0], uvs[1], col, vMap));
	indices[10] = mesh.AddVertex(Create<Vertex>(positions[6], normals[0], uvs[2], col, vMap));
	indices[11] = mesh.AddVertex(Create<Vertex>(positions[2], normals[0], uvs[3], col, vMap));
	// Right
	indices[12] = mesh.AddVertex(Create<Vertex>(positions[3], normals[1], uvs[0], col, vMap));
	indices[13] = mesh.AddVertex(Create<Vertex>(positions[7], normals[1], uvs[1], col, vMap));
	indices[14] = mesh.AddVertex(Create<Vertex>(positions[5], normals[1], uvs[2], col, vMap));
	indices[15] = mesh.AddVertex(Create<Vertex>(positions[1], normals[1], uvs[3], col, vMap));

	// Front
	indices[16] = mesh.AddVertex(Create<Vertex>(positions[2], normals[3], uvs[0], col, vMap));
	indices[17] = mesh.AddVertex(Create<Vertex>(positions[6], normals[3], uvs[1], col, vMap));
	indices[18] = mesh.AddVertex(Create<Vertex>(positions[7], normals[3], uvs[2], col, vMap));
	indices[19] = mesh.AddVertex(Create<Vertex>(positions[3], normals[3], uvs[3], col, vMap));
	// Back
	indices[20] = mesh.AddVertex(Create<Vertex>(positions[1], normals[2], uvs[0], col, vMap));
	indices[21] = mesh.AddVertex(Create<Vertex>(positions[5], normals[2], uvs[1], col, vMap));
	indices[22] = mesh.AddVertex(Create<Vertex>(positions[4], normals[2], uvs[2], col, vMap));
	indices[23] = mesh.AddVertex(Create<Vertex>(positions[0], normals[2], uvs[3], col, vMap));

	#pragma endregion

	#pragma region Make Triangles

	for (int ix = 0; ix < 6; ix++) {
		size_t o = ix * 4;
		mesh.AddIndexTri(indices[o + 0], indices[o + 1], indices[o + 2]);
		mesh.AddIndexTri(indices[o + 0], indices[o + 2], indices[o + 3]);
	}

	#pragma endregion
}

template <typename Vertex>
void MeshFactory::AddParameterized(MeshBuilder<Vertex>& mesh, const MeshBuilderParam& param) {
	const std::unordered_map<std::string, glm::vec3>& Params = param.Params;
	switch (param.Type) {
		case MeshBuilderType::Plane:
			MeshFactory::AddPlane(mesh, Params.at("position"), Params.at("normal"), Params.at("tangent"), Params.at("scale"), Params.at("uv_scale"), param.Color);
			break;
		case MeshBuilderType::Cube:
			MeshFactory::AddCube(mesh, Params.at("position"), Params.at("scale"), Params.at("rotation"), param.Color);
			break;
		case MeshBuilderType::IcoShere:
			MeshFactory::AddIcoSphere(mesh, Params.at("position"), Params.at("radius"), Params.at("tessellation").x, param.Color);
			break;
		case MeshBuilderType::UvSphere:
			MeshFactory::AddUvSphere(mesh, Params.at("position"), Params.at("radius"), Params.at("tessellation").x, param.Color);
			break;
		case MeshBuilderType::FaceInvert:
			MeshFactory::InvertFaces(mesh);
		default:
			break;
	}
}

template <typename Vertex>
void MeshFactory::InvertFaces(MeshBuilder<Vertex>& mesh)
{
	if (mesh.GetIndexCount() > 0) {
		uint32_t* data = (uint32_t*)mesh.GetIndexDataPtr();
		for (size_t ix = 0; ix < mesh.GetIndexCount(); ix+=3) {
			//https://www.geeksforgeeks.org/swap-two-numbers-without-using-temporary-variable/
			data[ix]     = data[ix] + data[ix + 1];
			data[ix + 1] = data[ix] - data[ix + 1];
			data[ix]     = data[ix] - data[ix + 1];
		}
	} else if (mesh.GetVertexCount() > 0) {
		Vertex* data = (Vertex*)mesh.GetVertexDataPtr();
		for (size_t ix = 0; ix < mesh.GetVertexCount(); ix+=3) {
			Vertex temp = data[ix];
			data[ix] = data[ix + 1];
			data[ix + 1] = temp;
		}
	}
}


template <typename Vertex>
void MeshFactory::CalculateTBN(MeshBuilder<Vertex>& mesh)
{
	VertexParamMap vMap = VertexParamMap(Vertex::V_DECL);
	if (vMap.TangentOffset == -1 && vMap.BiTangentOffset == -1) {
		LOG_WARN("Vertex type does not have tangent or bitangent attribute, aborting CalculateTBN");
		return;
	}
	if (vMap.PositionOffset == -1 || vMap.TextureOffset == -1) {
		LOG_WARN("Vertex type does not required position and texture attributes, aborting CalculateTBN");
		return;
	}
	if ( mesh._indices.size() == 0) {
		LOG_WARN("Mesh does not have indices, aborting CalculateTBN");
		return;
	}

	for (size_t i = 0; i < mesh._vertices.size(); i++) {
		vMap.SetTangent(mesh._vertices[i], glm::vec3(0.0f));
		vMap.SetBiTangent(mesh._vertices[i], glm::vec3(0.0f));
	}

	// Iterate over all indices in the mesh, we'll assume that the mesh is indexed
	for (size_t i = 0; i < mesh._indices.size(); i += 3) {
		Vertex& v1 = mesh._vertices[mesh._indices[i + 0u]];
		Vertex& v2 = mesh._vertices[mesh._indices[i + 1u]];
		Vertex& v3 = mesh._vertices[mesh._indices[i + 2u]];

		// Extract the positions from the mesh
		glm::vec3 pos[3];
		pos[0] = vMap.GetPosition(v1);
		pos[1] = vMap.GetPosition(v2);
		pos[2] = vMap.GetPosition(v3);

		// Extract the UVs from the mesh
		glm::vec2 uvs[3];
		uvs[0] = vMap.GetTexture(v1);
		uvs[1] = vMap.GetTexture(v2);
		uvs[2] = vMap.GetTexture(v3);

		// Calculate 2 corner vectors
		glm::vec3 deltaP1 = pos[1] - pos[0];
		glm::vec3 deltaP2 = pos[2] - pos[0];

		// Calculate UV deltas
		glm::vec2 deltaT1 = uvs[1] - uvs[0];
		glm::vec2 deltaT2 = uvs[2] - uvs[0];

		// Use the deltas in position and UV to calculate the tangent and bitangent
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		float r = 1.0f / (deltaT1.x * deltaT2.y - deltaT1.y * deltaT2.x);
		glm::vec3 tangent = glm::normalize((deltaP1 * deltaT2.y - deltaP2 * deltaT1.y) * r);
		glm::vec3 bitangent = glm::normalize((deltaP2 * deltaT1.x - deltaP1 * deltaT2.x) * r);

		// Set attributes in the vertex
		vMap.SetTangent(v1, glm::normalize((vMap.GetTangent(v1) + tangent) / 2.0f));
		vMap.SetTangent(v2, glm::normalize((vMap.GetTangent(v2) + tangent) / 2.0f));
		vMap.SetTangent(v3, glm::normalize((vMap.GetTangent(v3) + tangent) / 2.0f));

		vMap.SetBiTangent(v1, glm::normalize((vMap.GetBiTangent(v1) + bitangent) / 2.0f));
		vMap.SetBiTangent(v2, glm::normalize((vMap.GetBiTangent(v1) + bitangent) / 2.0f));
		vMap.SetBiTangent(v3, glm::normalize((vMap.GetBiTangent(v1) + bitangent) / 2.0f));
	}
}