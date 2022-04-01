#pragma once
#include <vector>
#include "Graphics/VertexArrayObject.h"

/// <summary>
/// A utility class that lets us add vertices and indices, then bake it into a final mesh, using interleaved
/// vertex buffers
/// </summary>
/// <typeparam name="VertType">The type of vertex that this mesh is using</typeparam>
template <typename VertType>
class MeshBuilder
{
public:
	MeshBuilder() :
		_vertices(std::vector<VertType>()),
		_indices(std::vector<uint32_t>()) {}
	~MeshBuilder() = default;

	/// <summary>
	/// Adds a new vertex to the mesh, returning it's index within the vertex buffer
	/// </summary>
	/// <param name="vertex">The vertex to add to the buffer</param>
	/// <returns>The index of the vertex, as can be added to an index buffer</returns>
	uint32_t AddVertex(const VertType& vertex) {
		_vertices.push_back(vertex);
		return static_cast<uint32_t>(_vertices.size() - 1u);
	}

	/// <summary>
	/// Adds a range of vertices to this mesh
	/// </summary>
	/// <param name="data">The array of vertices to add to this mesh</param>
	/// <param name="count">The number of verties in data</param>
	/// <returns>The starting index in the mesh for the range of data</returns>
	uint32_t AddVertexRange(const VertType* data, uint32_t count) {
		uint32_t index = static_cast<uint32_t>(_vertices.size());
		// Reserve space for the incoming vertices, ensures the underlying datastore will be large enough
		_vertices.reserve(_vertices.size() + count);
		// Copy data into the container, this will also update the container's size!
		std::copy(data, data + count, std::back_inserter(_vertices));
		// Return the index of the start of the range
		return index;
	}
	/// <summary>
	/// Adds a range of vertices to this mesh
	/// </summary>
	/// <param name="data">The array of vertices to add to this mesh</param>
	/// <returns>The starting index in the mesh for the range of data</returns>
	uint32_t AddVertexRange(const std::vector<VertType>& data) {
		return AddVertexRange(data.data(), data.size());
	}

	/// <summary>
	/// Constructs and adds a new vertex, passing the parameters to the vertex's constructor
	/// </summary>
	/// <typeparam name="...Args">The types of parameters to forward</typeparam>
	/// <param name="...args">The arguments to forward to the vertex constructor</param>
	/// <returns>The index of the vertex, as can be added to an index buffer</returns>
	template<class...Args>
	uint32_t AddVertex(Args&&... args) {
		_vertices.emplace_back(std::forward<Args>(args)...);
		return static_cast<uint32_t>(_vertices.size() - 1u);
	}
	
	/// <summary>
	/// Adds an index to the index buffer
	/// </summary>
	/// <param name="index">The index to append to the buffer</param>
	void AddIndex(uint32_t index) {
		_indices.push_back(index);
	}

	/// <summary>
	/// Adds a triangle between the three indices
	/// </summary>
	/// <param name="a">The index of the first vertex</param>
	/// <param name="b">The index of the second vertex</param>
	/// <param name="c">The index of the third vertex</param>
	void AddIndexTri(uint32_t a, uint32_t b, uint32_t c)
	{
		ReserveIndexSpace(3);
		_indices.push_back(a);
		_indices.push_back(b);
		_indices.push_back(c);
	}
	
	/// <summary>
	/// Resizes the internal vector to allocate space for new vertices, can improve
	/// performance when appending large meshes of a known size
	/// </summary>
	/// <param name="extendAmount">The number of vertices to reserve space for</param>
	void ReserveVertexSpace(size_t extendAmount) {
		_vertices.reserve(_vertices.size() + extendAmount);
	}
	/// <summary>
	/// Resizes the internal vector to allocate space for new indices, can improve
	/// performance when appending large meshes of a known size
	/// </summary>
	/// <param name="extendAmount">The number of indices to reserve space for</param>
	void ReserveIndexSpace(size_t extendAmount) {
		_indices.reserve(_indices.size() + extendAmount);
	}

	/// <summary>
	/// Returns the number of vertices in this mesh
	/// </summary>
	size_t GetVertexCount() const { return _vertices.size(); }
	/// <summary>
	/// Returns the number of indices in this mesh
	/// </summary>
	size_t GetIndexCount() const { return _indices.size(); }
	/// <summary>
	/// Returns the number of triangles in this mesh. If the index vector contains data,
	/// it will calculate the triangle count using that, otherwise it will use the number
	/// of vertices
	/// </summary>
	size_t GetTriangleCount() const { return _indices.size() > 0 ? _indices.size() / 3 : _vertices.size() / 3; }

	/// <summary>
	/// Creates and returns a VertexArraybject from the current data
	/// </summary>
	/// <returns>A VertexArrayObject</returns>
	VertexArrayObject::Sptr Bake() {
		VertexBuffer::Sptr vbo = VertexBuffer::Create();
		vbo->LoadData(GetVertexDataPtr(), _vertices.size());

		IndexBuffer::Sptr ebo = nullptr;
		if (_indices.size() > 0) {
			ebo = IndexBuffer::Create();
			ebo->LoadData(GetIndexDataPtr(), _indices.size());
		}

		// Create VAO and attach the buffers
		VertexArrayObject::Sptr result = VertexArrayObject::Create();
		result->AddVertexBuffer(vbo, VertType::V_DECL);
		result->SetIndexBuffer(ebo);

		// Store our vertex type in the VAO's vertex declaration
		result->SetVDecl(VertType::V_DECL);

		return result;
	}
	
	/// <summary>
	/// Resets this mesh, removing all vertices and indices
	/// </summary>
	void Reset() {
		_vertices.clear();
		_indices.clear();
	}

	/// <summary>
	/// Gets a pointer to the underlying vertex data in the mesh, valid only
	/// until another call to AddVertex
	/// </summary>
	const VertType* GetVertexDataPtr() const {
		return _vertices.data();
	}
	/// <summary>
	/// Gets a pointer to the underlying index data in the mesh, valid only
	/// until another call to AddIndex or AddIndexTri
	/// </summary>
	const uint32_t* GetIndexDataPtr() const {
		return _indices.data();
	}
	
protected:
	friend class MeshFactory;
	
	std::vector<VertType> _vertices;
	std::vector<uint32_t> _indices;
};
