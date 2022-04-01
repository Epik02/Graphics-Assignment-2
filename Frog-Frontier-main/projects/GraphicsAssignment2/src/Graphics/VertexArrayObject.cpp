#include "VertexArrayObject.h"
#include "Buffers/IndexBuffer.h"
#include "Buffers/VertexBuffer.h"
#include "Logging.h"

VertexArrayObject::VertexArrayObject() :
	_indexBuffer(nullptr),
	_handle(0),
	_vertexCount(0),
	_elementCount(0),
	_vertexBuffers(std::vector<VertexBufferBinding*>())
{
	glCreateVertexArrays(1, &_handle);
}

VertexArrayObject::~VertexArrayObject()
{
	if (_handle != 0) {
		glDeleteVertexArrays(1, &_handle);
		_handle = 0;
	}
}

void VertexArrayObject::SetIndexBuffer(const IndexBuffer::Sptr& ibo) {
	// TODO: What if we already have a buffer? should we delete it? who owns the buffer?
	_indexBuffer = ibo;
	Bind();
	if (_indexBuffer != nullptr) {
		_indexBuffer->Bind();
		_elementCount = _indexBuffer->GetElementCount();
	}
	else {
		IndexBuffer::Unbind();
		_elementCount = _vertexCount;
	}
	Unbind();
}

VertexArrayObject::VertexBufferBinding* VertexArrayObject::AddVertexBuffer(const VertexBuffer::Sptr& buffer, const std::vector<BufferAttribute>& attributes, bool instanced) {
	if (_vertexBuffers.size() == 0) {
		_vertexCount = buffer->GetElementCount();
		if (_indexBuffer == nullptr) {
			_elementCount = _vertexCount;
		}
	} 
	else if (buffer->GetElementCount() != _vertexCount) {
		LOG_WARN("Buffer element count does not match vertex count of this VAO!!!");
	}

	VertexBufferBinding* binding = new VertexBufferBinding();
	binding->Buffer = buffer;
	binding->Attributes = attributes;
	binding->Instanced = instanced;
	_vertexBuffers.push_back(binding);


	Bind();
	buffer->Bind();
	for (const BufferAttribute& attrib : attributes) {
		glEnableVertexArrayAttrib(_handle, attrib.Slot);
		glVertexAttribPointer(attrib.Slot, attrib.Size, (GLenum)attrib.Type, attrib.Normalized, attrib.Stride,
							  (void*)attrib.Offset);

		// Here is where we select whether the attribute is instanced or not
		glVertexAttribDivisor(attrib.Slot, instanced ? 1 : 0);
	}
	Unbind();

	return binding;
}

void VertexArrayObject::ReplaceVertexBuffer(VertexBufferBinding* binding, const VertexBuffer::Sptr& buffer)
{
	// Search for the BufferAttribute with the matching usage
	auto& it = std::find_if(_vertexBuffers.begin(), _vertexBuffers.end(), [&](const VertexBufferBinding* buffer) {
		return buffer == binding;
	});

	if (it != _vertexBuffers.end()) {
		if (buffer->GetElementCount() != _vertexCount) {
			LOG_WARN("Buffer element count does not match vertex count of this VAO!!!");
		}

		// Update the buffer the binding is pointing to
		binding->Buffer = buffer;

		// Re-bind the buffer and attributes
		Bind();
		buffer->Bind();
		for (const BufferAttribute& attrib : binding->Attributes) {
			glEnableVertexArrayAttrib(_handle, attrib.Slot);
			glVertexAttribPointer(attrib.Slot, attrib.Size, (GLenum)attrib.Type, attrib.Normalized, attrib.Stride,
				(void*)attrib.Offset);

			// Here is where we select whether the attribute is instanced or not
			glVertexAttribDivisor(attrib.Slot, binding->Instanced ? 1 : 0);
		}
		Unbind();
	}


}

void VertexArrayObject::Draw(DrawMode mode) {
	Bind();
	if (_indexBuffer == nullptr) {
		uint32_t elements = _elementCount == 0 ? _vertexBuffers[0]->Buffer->GetElementCount() : _elementCount;
		glDrawArrays((GLenum)mode, 0, elements);
	} else {
		uint32_t elements = _elementCount == 0 ? _indexBuffer->GetElementCount() : _elementCount;
		glDrawElements((GLenum)mode, elements, (GLenum)_indexBuffer->GetElementType(), nullptr);
	}
	Unbind();
}

void VertexArrayObject::DrawInstanced(uint32_t instanceCount, DrawMode mode /*= DrawMode::TriangleList*/)
{
	Bind();
	if (_indexBuffer == nullptr) {
		uint32_t elements = _elementCount == 0 ? _vertexBuffers[0]->Buffer->GetElementCount() : _elementCount;
		glDrawArraysInstanced((GLenum)mode, 0, elements, instanceCount);
	}
	else {
		uint32_t elements = _elementCount == 0 ? _indexBuffer->GetElementCount() : _elementCount;
		glDrawElementsInstanced((GLenum)mode, elements, (GLenum)_indexBuffer->GetElementType(), nullptr, instanceCount);
	}
	Unbind();
	
}

void VertexArrayObject::Bind() {
	glBindVertexArray(_handle);
}

void VertexArrayObject::Unbind() {
	glBindVertexArray(0);
}

void VertexArrayObject::SetVDecl(const VertexDeclaration& vDecl) {
	_vDecl = vDecl;
}

const VertexArrayObject::VertexDeclaration& VertexArrayObject::GetVDecl() {
	return _vDecl;
}

GlResourceType VertexArrayObject::GetResourceClass() const {
	return GlResourceType::VertexArray;
}

VertexArrayObject::VertexBufferBinding* VertexArrayObject::GetBufferBinding(AttribUsage usage) {
	for (auto& binding : _vertexBuffers) {
		// Search for the BufferAttribute with the matching usage
		auto& it = std::find_if(binding->Attributes.begin(), binding->Attributes.end(), [&](const BufferAttribute& attrib) {
			return attrib.Usage == usage;
		});

		// If the BufferAttribute was found, return the binding
		if (it != binding->Attributes.end()) {
			return binding;
		}
	}
	return nullptr;
}

VertexArrayObject::Sptr VertexArrayObject::Clone() const
{
	VertexArrayObject::Sptr result = Create();
	result->SetDebugName(GetDebugName() + " - clone");

	if (_indexBuffer != nullptr) {
		result->SetIndexBuffer(_indexBuffer);
	}

	for (const auto& binding : _vertexBuffers) {
		result->AddVertexBuffer(binding->Buffer, binding->Attributes, binding->Instanced);
	}

	result->SetVDecl(_vDecl);

	return result;
}

