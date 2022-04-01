#pragma once

#include <EnumToString.h>
#include <GLM/glm.hpp>
#include <unordered_map>
#include <vector>

#include "Utils/ResourceManager/IResource.h"
#include "Utils/Macros.h"
#include "glad/glad.h"
#include "Graphics/IGraphicsResource.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/GlEnums.h"

/**
 * Describes a render buffer that may be attached to a Framebuffer
 */
struct RenderTargetDescriptor {
	/**
	 * True if the render target should use a texture instead of a render buffer.
	 * Textures can be accessed by shaders for post processing, but may be slower
	 * than using renderbuffers
	 */
	bool                   UseTexture = true;
	/**
	 * The format that the rendertarget should use
	 */
	RenderTargetType       Format;

	RenderTargetDescriptor(RenderTargetType format = RenderTargetType::ColorRgba8, bool useTexture = true) :
		UseTexture(useTexture),
		Format(format) 
	{ }
};

/**
 * Describes a frame buffer, allowing framebuffers to be easily serialized
 * and cloned
 */
struct FramebufferDescriptor {
	uint32_t Width;
	uint32_t Height;
	std::unordered_map<RenderTargetAttachment, RenderTargetDescriptor> RenderTargets;

	FramebufferDescriptor() :
		Width(0),
		Height(0),
		RenderTargets(std::unordered_map<RenderTargetAttachment, RenderTargetDescriptor>())
	{ }
};

/**
 * Wraps around an OpenGL framebuffer, allowing us to render to textures or render buffers
 */
class Framebuffer : public IGraphicsResource, public IResource {
public:
	DEFINE_RESOURCE(Framebuffer)

	Framebuffer(const FramebufferDescriptor& description);
	~Framebuffer();

	/**
	 * Gets the width of the framebuffer and its attachments in pixels
	 */
	uint32_t GetWidth() const;
	/**
	 * Gets the height of the framebuffer and its attachments in pixels
	 */
	uint32_t GetHeight() const;
	/**
	 * Gets the dimensions of the framebuffer and its attachments in pixels
	 */
	glm::ivec2 GetSize() const;

	/**
	 * Gets the texture attached to the given render target attachment, or nullptr
	 * if the attachment is empty or is a RenderBuffer
	 * 
	 * @param attachment The render target attachment slot to fetch
	 * @returns The texture bound to the given slot, or nullptr if the attachment is empty or a renderbuffer
	 */
	Texture2D::Sptr GetTextureAttachment(RenderTargetAttachment attachment) const;

	/**
	 * Resizes this Framebuffer and all attachments to the given dimensions in pixels. Destroys all data
	 * in the rendertargets
	 * 
	 * @param width The new width for the framebuffer in pixels
	 * @param height The new height for the framebuffer in pixels
	 */
	void Resize(uint32_t width, uint32_t height);
	/**
	 * Resizes this Framebuffer and all attachments to the given dimensions in pixels. Destroys all data
	 * in the rendertargets
	 * 
	 * @param size The new size of the framebuffer in pixels
	 */
	void Resize(const glm::ivec2& size);

	/**
	 * Validates the framebuffer and returns true if it is ready for use in rendering
	 */
	bool Validate();

	/**
	 * Binds the texture at the given render target attachment to the given texture slot
	 * If the attachment is not defined or is not a texture, no changes will be made
	 * 
	 * @param attachment The render target attachment point to bind
	 * @param slot       The texture slot to bind to 
	 */
	bool BindAttachment(RenderTargetAttachment attachment, int slot) const;

	/**
	 * Binds this framebuffer to the given framebuffer binding slot
	 * 
	 * @param bindMode The binding slot to bind to (default is GL_DRAW)
	 */
	void Bind(FramebufferBinding bindMode = FramebufferBinding::Draw) const;
	/**
	 * Unbinds the framebuffer from the given binding slot, resetting it to the default framebuffer
	 *
	 * @param bindMode The binding slot to bind to (default is GL_DRAW)
	 */
	void Unbind();

	/**
	 * Copies the contents of one framebuffer to another
	 * This will filter the attachments if the buffers are different sizes, as well as resolving multisampling
	 * 
	 * @param source The source framebuffer to read data from
	 * @param dest   The destination framebuffer to write data to
	 * @param flags  The buffer attachments that should be copied (color, depth, stencil)
	 * @param filter The texture filtering to use if the source and destination buffers are not the same size
	 */
	static void Blit(const Framebuffer::Sptr& source, const Framebuffer::Sptr& dest, BufferFlags flags = BufferFlags::All, MagFilter filter = MagFilter::Linear);
	/**
	 * Copies the contents of the framebuffer bound using Read to the contents bound using Write or Draw
	 * This will filter the attachments if the buffers are different sizes, as well as resolving multisampling
	 * 
	 * @param srcBounds The bounds within the source framebuffer to copy, as a set of 2 points (x1, y1, x2, y2)
	 * @param dstBounds The bounds within the destination framebuffer to copy to, as a set of 2 points (x1, y1, x2, y2)
	 * @param flags  The buffer attachments that should be copied (color, depth, stencil)
	 * @param filter The texture filtering to use if the source and destination buffers are not the same size
	 */
	static void Blit(const glm::ivec4& srcBounds, const glm::ivec4& dstBounds, BufferFlags flags = BufferFlags::All, MagFilter filter = MagFilter::Linear);

	/**
	 * Creates a copy of this framebuffer's configuration. Note that this copy will
	 * contain empty textures and renderbuffers
	 */
	Sptr Clone();

	// Inherited from IGraphicsResource

	virtual void SetDebugName(const std::string& name) override;
	virtual GlResourceType GetResourceClass() const override;

	// Inherited from IResource

	virtual nlohmann::json ToJson() const override;
	static Framebuffer::Sptr FromJson(const nlohmann::json& blob);

protected:
	// The descriptor for this framebuffer
	FramebufferDescriptor _description;
	// True if the Framebuffer has been validated
	bool                  _isValid;

	// The slots that the framebuffer is currently bound to
	mutable FramebufferBinding _currentBinding;

	// Describes a single render target within the framebuffer
	struct RenderTarget {
		// The texture or RenderBuffer for the target
		IGraphicsResource::Sptr Resource;
		// True if the resource is a renderbuffer, false for textures
		bool                    IsRenderBuffer;
		// The descriptor for this render target
		RenderTargetDescriptor  Description;

		RenderTarget();
	};

	typedef std::unordered_map<RenderTargetAttachment, RenderTarget> __TargetMap;
	__TargetMap _targets;

	std::vector<RenderTargetAttachment> _drawBuffers;

	void _AddAttachment(RenderTargetAttachment attachment, const RenderTargetDescriptor& target);
};

