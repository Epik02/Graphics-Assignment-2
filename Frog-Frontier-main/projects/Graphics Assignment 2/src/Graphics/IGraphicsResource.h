#pragma once

#include <string>
#include <cstdint>
#include <glad/glad.h>
#include <EnumToString.h>

#include "Utils/ResourceManager/IResource.h"
#include "Utils/Macros.h"

/**
 * Enumerates all OpenGL resource types, as can be passed into glObjectLabel and other 
 * utility functions in OpenGL
 */
ENUM(GlResourceType, GLenum,
	 Buffer            = GL_BUFFER,
	 ShaderProgram            = GL_SHADER,
	 Program           = GL_PROGRAM,
	 VertexArray       = GL_VERTEX_ARRAY,
	 Query             = GL_QUERY,
	 ProgramPipeline   = GL_PROGRAM_PIPELINE,
	 TransformFeedback = GL_TRANSFORM_FEEDBACK,
	 Sampler           = GL_SAMPLER,
	 Texture           = GL_TEXTURE,
	 RenderBuffer      = GL_RENDERBUFFER,
	 FrameBuffer       = GL_FRAMEBUFFER,
	 Unknown           = GL_NONE
)

/**
 * Base class for all of our OpenGL graphics resources 
 */
class IGraphicsResource {
public:
	// For pointers and deletion of move and copy
	DEFINE_RESOURCE(IGraphicsResource)

	virtual ~IGraphicsResource() = default;

	/**
	 * Should be overridden in derived classes to return a resource type identifier
	 */
	virtual GlResourceType GetResourceClass() const = 0;

	/**
	 * Sets a debug name for this resource. This will appear in some debug
	 * messages as well as when using RenderDoc or other debuggers
	 * @param name The name for the resource
	 */
	virtual void SetDebugName(const std::string& name);
	/**
	 * Gets the debug name for this resource
	 */
	virtual const std::string& GetDebugName() const;

	/**
	 * Returns the underlying OpenGL handle of this resource
	 */
	virtual uint32_t GetHandle() const;

protected:
	IGraphicsResource();
	
	/**
	 * Updates the underlying render ID and ensures that the resources debug name
	 * is accurate. Should be used instead of setting _rendererId directly
	 */
	void _SetRenderId(uint32_t renderId);

	std::string _debugName;
	uint32_t    _rendererId;
};