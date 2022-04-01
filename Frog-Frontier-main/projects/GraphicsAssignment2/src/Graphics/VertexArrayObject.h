#pragma once
#include <glad/glad.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <EnumToString.h>

#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/GlEnums.h"
#include "Graphics/IGraphicsResource.h"

/// <summary>
/// This structure will represent the parameters passed to the glVertexAttribPointer commands
/// </summary>
struct BufferAttribute {
	/// <summary>
	/// The input slot to the vertex shader that will receive the data
	/// </summary>
	GLuint  Slot;
	/// <summary>
	/// The number of elements to be passed (ex 3 for a vec3)
	/// </summary>
	GLint   Size;
	/// <summary>
	/// The type of data to be passed (ex: GL_FLOAT for a vec3)
	/// </summary>
	AttributeType  Type;
	/// <summary>
	/// Specifies whether fixed-point data values should be normalized (true) or converted directly as fixed-point values (false) when they are accessed. Usually false
	/// </summary>
	bool    Normalized;
	/// <summary>
	/// The total size of an element in this buffer
	/// </summary>
	GLsizei Stride;
	/// <summary>
	/// The offset from the start of an element to this attribute
	/// </summary>
	GLsizei Offset;
	/// <summary>
	/// A hint for how the vertex attribute may be used (useful for our own code)
	/// </summary>
	AttribUsage Usage;

	BufferAttribute() :
		Slot(0), Size(0), Type(AttributeType::Unknown), Normalized(false), Stride(0), Offset(0), Usage(AttribUsage::Unknown){}

	BufferAttribute(uint32_t slot, uint32_t size, AttributeType type, GLsizei stride, GLsizei offset, AttribUsage usage, bool normalized = false) :
		Slot(slot), Size(size), Type(type), Stride(stride), Offset(offset), Usage(usage), Normalized(normalized) { }
};

/// <summary>
/// The Vertex Array Object wraps around an OpenGL VAO and basically represents all of the data for a mesh
/// </summary>
class VertexArrayObject final : public IGraphicsResource
{
public:
	typedef std::vector<BufferAttribute> VertexDeclaration;
	DEFINE_RESOURCE(VertexArrayObject);

	static inline Sptr Create() {
		return std::make_shared<VertexArrayObject>();
	}

	// Helper structure to store a buffer and the attributes
	struct VertexBufferBinding {
		const VertexBuffer::Sptr& GetBuffer() const { return Buffer; }
		const std::vector<BufferAttribute>& GetAttributes() const { return Attributes; }
		bool IsInstanced() const { return Instanced; }

	protected:
		friend class VertexArrayObject;

		VertexBuffer::Sptr Buffer;
		std::vector<BufferAttribute> Attributes;
		bool Instanced;
	};
	
public:
	/// <summary>
	/// Creates a new empty Vertex Array Object
	/// </summary>
	VertexArrayObject();
	// Destructor does not need to be virtual due to the use of the final keyword
	~VertexArrayObject();

	uint32_t GetVertexCount() const { return _vertexCount; }
	uint32_t GetIndexCount() const { return _indexBuffer != nullptr ? _indexBuffer->GetElementCount() : 0; }
	uint32_t GetElementCount() const { return _elementCount; }

	/// <summary>
	/// Creates a copy of this VAO pointing to the same buffers, with the same attributes
	/// </summary>
	/// <returns>A duplicate VAO</returns>
	Sptr Clone() const;

	/// <summary>
	/// Sets the index buffer for this VAO, note that for now, this will not delete the buffer when the VAO is deleted, more on that later
	/// </summary>
	/// <param name="ibo">The index buffer to bind to this VAO</param>
	void SetIndexBuffer(const IndexBuffer::Sptr& ibo);
	IndexBuffer::Sptr GetIndexBuffer() const { return _indexBuffer; }

	/// <summary>
	/// Adds a vertex buffer to this VAO, with the specified attributes
	/// </summary>
	/// <param name="buffer">The buffer to add (note, does not take ownership, you will still need to delete later)</param>
	/// <param name="attributes">A list of vertex attributes that will be fed by this buffer</param>
	/// <param name="instanced">True if the buffer should contain one set of data per instance, false for per vertex</param>
	VertexBufferBinding* AddVertexBuffer(const VertexBuffer::Sptr& buffer, const std::vector<BufferAttribute>& attributes, bool instanced = false);

	void ReplaceVertexBuffer(VertexBufferBinding* binding, const VertexBuffer::Sptr& buffer);

	/// <summary>
	/// Gets the buffer binding that has an attribute with the given usage
	/// We can use this for extracting info from a VBO at a later time
	/// </summary>
	/// <param name="usage">The attribute usage hint to search for</param>
	/// <returns>A const pointer to the binding, or nullptr if none is found</returns>
	VertexBufferBinding* GetBufferBinding(AttribUsage usage);

	/// <summary>
	/// Renders this VAO, using the specified draw mode
	/// </summary>
	/// <param name="mode">The draw mode for primitives in this VAO</param>
	void Draw(DrawMode mode = DrawMode::TriangleList);

	/// <summary>
	/// Renders this VAO with the given instance count, using the specified draw mode. 
	/// Internally this will call glDrawArraysInstanced or glDrawElementsInstanced
	/// </summary>
	/// <param name="instanceCount">The number of instances to render</param>
	/// <param name="mode">The primitive mode for rendering the mesh</param>
	void DrawInstanced(uint32_t instanceCount, DrawMode mode = DrawMode::TriangleList);

	/// <summary>
	/// Binds this VAO as the source of data for draw operations
	/// </summary>
	void Bind();
	/// <summary>
	/// Unbinds the currently bound VAO
	/// </summary>
	static void Unbind();

	/// <summary>
	/// Returns the underlying OpenGL handle that this class is wrapping around
	/// </summary>
	GLuint GetHandle() const { return _handle; }

	void SetVDecl(const VertexDeclaration& vDecl);
	const VertexDeclaration& GetVDecl();

protected:
	
	// The index buffer bound to this VAO
	IndexBuffer::Sptr _indexBuffer;
	// The vertex buffers bound to this VAO
	std::vector<VertexBufferBinding*> _vertexBuffers;

	// Stores a copy of one of the vertex declarations
	// defined in VertexTypes.cpp
	VertexDeclaration _vDecl;

	uint32_t _vertexCount;
	uint32_t _elementCount;

	// The underlying OpenGL handle that this class is wrapping around
	GLuint _handle;

	// Inherited via IGraphicsResource
	virtual GlResourceType GetResourceClass() const override;
};
