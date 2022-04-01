#pragma once
#include "IBuffer.h"
#include <memory>

/// <summary>
/// A uniform buffer that operates on raw data
/// Since this is a base class for the typed Uniform Buffers,
/// we can use the shared ptr to store uniform buffers with
/// different structures
/// </summary>
class AbstractUniformBuffer : public IBuffer {
public :
	typedef std::shared_ptr<AbstractUniformBuffer> Sptr;

	virtual ~AbstractUniformBuffer();

	/// <summary>
	/// Creates a new raw data uniform buffer with the given size in bytes
	/// and usage
	/// </summary>
	/// <param name="sizeInBytes">The size in bytes for the buffer</param>
	/// <param name="usage">The buffer's usage hint, default DynamicDraw</param>
	AbstractUniformBuffer(uint32_t sizeInBytes, BufferUsage usage = BufferUsage::DynamicDraw);
	/// <summary>
	/// Loads fresh data into this uniform buffer
	/// </summary>
	/// <param name="data">The data to load into the buffer</param>
	/// <param name="elementSize">The size of a single element in bytes</param>
	/// <param name="elementCount">The numbder of elements to upload</param>
	virtual void LoadData(const void* data, uint32_t elementSize, uint32_t elementCount) override;

	/// <summary>
	/// Uniform buffers behave a bit differently than other buffer types,
	/// so we need to have a custom bind command, by default will bind to
	/// slot 0
	/// </summary>
	void Bind() const override;
	/// <summary>
	/// Binds this UBO to the specified binding slot
	/// </summary>
	/// <param name="slot">The buffer binding slot to bind to</param>
	void Bind(int slot) const;

protected:
	// Will contain the backing data store for the buffer
	uint8_t* _rawData;
	uint32_t _size;
};

/// <summary>
/// Defines a uniform buffer that wraps around a C++ structure
/// that has been designed to match the GLSL structure
/// </summary>
/// <typeparam name="Structure">The type of the C++ structure</typeparam>
template <typename Structure>
class UniformBuffer : public AbstractUniformBuffer {
public:
	typedef std::shared_ptr<UniformBuffer<Structure>> Sptr;

	/// <summary>
	/// Creates a new Uniform Buffer with the given usage
	/// </summary>
	/// <param name="usage">The buffer's usage hint, default DynamicDraw</param>
	UniformBuffer(BufferUsage usage = BufferUsage::DynamicDraw) :
		AbstractUniformBuffer(sizeof(Structure), usage) 
	{}
	/// <summary>
	/// Creates a new Uniform Buffer with the given usage
	/// Provides a value for the structure to copy into the buffer
	/// </summary>
	/// <param name="usage">The buffer's usage hint, default DynamicDraw</param>
	UniformBuffer(const Structure& data, BufferUsage usage = BufferUsage::DynamicDraw) :
		AbstractUniformBuffer(sizeof(Structure), usage) 
	{
		SetData(data);
	}

	/// <summary>
	/// Gets the data structure that this uniform buffer is
	/// using for storage
	/// </summary>
	/// <returns>A reference to the underlying data for this buffer</returns>
	Structure& GetData() {
		return *reinterpret_cast<Structure*>(_rawData);
	}
	/// <summary>
	/// Gets the data structure that this uniform buffer is
	/// using for storage
	/// </summary>
	/// <returns>A const reference to the underlying data for this buffer</returns>
	const Structure& GetData() const {
		return *reinterpret_cast<Structure*>(_rawData);
	}

	/// <summary>
	/// Replaces this buffer's data with the given data structure, and updates 
	/// OpenGL with the new data
	/// </summary>
	/// <param name="data">The new data to store in the UBO</param>
	void SetData(const Structure& data) {
		*((Structure*)_rawData) = data;
		Update();
	}

	/// <summary>
	/// Notifies OpenGL that the data has been updated and requires
	/// a resync with the GL side buffer
	/// </summary>
	void Update() {
		glNamedBufferSubData(_rendererId, 0, sizeof(Structure), _rawData);
	}
};
