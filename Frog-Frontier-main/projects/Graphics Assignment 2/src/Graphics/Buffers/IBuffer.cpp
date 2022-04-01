#include "IBuffer.h"
#include "Logging.h"

IBuffer::IBuffer(BufferType type, BufferUsage usage) :
	IGraphicsResource(),
	_elementCount(0),
	_elementSize(0),
	_size(0)
{
	_type = type;
	_usage = usage;
	glCreateBuffers(1, &_rendererId);
}

GlResourceType IBuffer::GetResourceClass() const {
	return GlResourceType::Buffer;
}

IBuffer::~IBuffer() {
	if (_rendererId != 0) {
		glDeleteBuffers(1, &_rendererId);
		_rendererId = 0;
	}
}

void IBuffer::LoadData(const void* data, uint32_t elementSize, uint32_t elementCount) {
	// Note, this is part of the bindless state access stuff added in 4.5
	glNamedBufferData(_rendererId, (GLsizeiptr)elementSize * elementCount, data, (GLenum)_usage);

	_elementCount = elementCount;
	_elementSize = elementSize;
	_size = elementCount * elementSize;
}

void IBuffer::UpdateData(const void* data, uint32_t elementSize, uint32_t elementCount, bool allowResize /*= true*/)
{
	if (elementSize * elementCount > _size) {
		if (allowResize) {
			glNamedBufferData(_rendererId, (GLsizeiptr)elementSize * elementCount, data, (GLenum)_usage);

			LOG_INFO("Expanding buffer from {} bytes to {} bytes", _size, elementCount * elementSize);

			_elementCount = elementCount;
			_elementSize = elementSize;
			_size = elementCount * elementSize;
		} else {
			LOG_ASSERT(false, "Attempting to write beyond the end of the buffer!");
		}
	} else {
		if (_size == 0) {
			glNamedBufferData(_rendererId, (GLsizeiptr)elementSize * elementCount, data, (GLenum)_usage);
			_size = elementCount * elementSize;
		} else {
			glNamedBufferSubData(_rendererId, 0, (GLsizeiptr)elementSize * elementCount, data);
		}
		_elementCount = elementCount;
		_elementSize = elementSize;

	}
}

void* IBuffer::Map(BufferMapMode mode) {
	return glMapNamedBufferRange(_rendererId, 0, _size, *mode);
}

void IBuffer::Unmap() {
	glUnmapNamedBuffer(_rendererId);
}

void IBuffer::Bind() const {
	glBindBuffer((GLenum)_type, _rendererId);
}

void IBuffer::Bind(uint32_t slot) const
{
	glBindBufferBase((GLenum)_type, slot, _rendererId);
}

void IBuffer::UnBind(BufferType type) {
	glBindBuffer((GLenum)type, 0);
}

void IBuffer::UnBind(BufferType type, uint32_t slot) {
	glBindBufferBase((GLenum)type, slot, 0);
}
