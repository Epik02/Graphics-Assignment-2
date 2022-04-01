#include "Graphics/Framebuffer.h"

#include "Graphics/RenderBuffer.h"
#include "Utils/JsonGlmHelpers.h"


Framebuffer::Framebuffer(const FramebufferDescriptor& description) :
	IGraphicsResource(),
	_description(FramebufferDescriptor()),
	_isValid(false),
	_targets(__TargetMap()),
	_drawBuffers(std::vector<RenderTargetAttachment>())
{
	_description = description;
	LOG_ASSERT(_description.Width * _description.Height > 0, "Width and height must both be > 0");

	// Generate the framebuffer
	glCreateFramebuffers(1, &_rendererId);

	// Create and attach all render targets
	for (const auto& kvp : _description.RenderTargets) {
		_AddAttachment(kvp.first, kvp.second);
	}
}

Framebuffer::~Framebuffer() {
	LOG_INFO("Deleting frame buffer with ID: {}", _rendererId);
	glDeleteFramebuffers(1, &_rendererId);
}

uint32_t Framebuffer::GetWidth() const {
	return _description.Width;
}

uint32_t Framebuffer::GetHeight() const {
	return _description.Height;
}

glm::ivec2 Framebuffer::GetSize() const {
	return glm::ivec2(_description.Width, _description.Height);
}

Texture2D::Sptr Framebuffer::GetTextureAttachment(RenderTargetAttachment attachment) const {
	// Find the attachment
	const auto& it = _targets.find(attachment);

	// If it exists, and is not a renderbuffer, cast to texture and return
	if (it != _targets.end() && !it->second.IsRenderBuffer) {
		return std::dynamic_pointer_cast<Texture2D>(it->second.Resource);
	}
	// Otherwise not found or is a renderbuffer, return nullptr
	else {
		return nullptr;
	}
}

void Framebuffer::Resize(uint32_t width, uint32_t height) {
	LOG_ASSERT(width * height > 0, "Width and height must be > 0");

	// We'll only do the heavy lifting if the dimensions have changed
	if (width != _description.Width || height != _description.Height) {
		// Update the description
		_description.Width  = width;
		_description.Height = height;

		// Re-attach all our rendertargets (releasing our references and re-creating them)
		for (const auto& kvp : _targets) {
			_AddAttachment(kvp.first, kvp.second.Description);
		}

		// Make sure we're good for rendering
		Validate();
	}
}

void Framebuffer::Resize(const glm::ivec2& size) {
	Resize(size.x, size.y);
}

// meat and potatoes
void Framebuffer::_AddAttachment(RenderTargetAttachment attachment, const RenderTargetDescriptor& target)
{
	// If the attachment already exists, release our reference to it's resource
	if (_targets.find(attachment) != _targets.end()) {
		_targets[attachment].Resource = nullptr;
	}
	// If this is a new attachment and is a color, add it to the draw buffers so OpenGL knows to render to it
	else if (IsColorAttachment(attachment)) {
		_drawBuffers.push_back(attachment);
		glNamedFramebufferDrawBuffers(_rendererId, _drawBuffers.size(), reinterpret_cast<GLenum*>(_drawBuffers.data()));
	}

	// Grab a reference to the rendertarget at that attachment
	// If one does not exist it will be created
	RenderTarget& buffer = _targets[attachment];
	buffer.Description = target;
	buffer.IsRenderBuffer = !target.UseTexture;

	// Handle creating render buffers 
	if (buffer.IsRenderBuffer) {
		RenderbufferDescription descriptor = RenderbufferDescription();
		// Per-framebuffer parameters
		descriptor.Width            = _description.Width;
		descriptor.Height           = _description.Height;

		// Per-attachment parameters
		descriptor.Format = target.Format;

		// Create and store the render buffer
		buffer.Resource = std::make_shared<Renderbuffer>(descriptor);
		glNamedFramebufferRenderbuffer(_rendererId, *attachment, GL_RENDERBUFFER, buffer.Resource->GetHandle());
	}
	// It's a texture
	else {
		Texture2DDescription descriptor = Texture2DDescription();
		// Per-framebuffer parameters
		descriptor.Width            = _description.Width;
		descriptor.Height           = _description.Height;

		// Per-attachment parameters
		descriptor.Format = (InternalFormat)target.Format;

		// Common parameters
		descriptor.GenerateMipMaps    = false;
		descriptor.MinificationFilter = MinFilter::Linear;
		descriptor.HorizontalWrap     = WrapMode::ClampToEdge;
		descriptor.VerticalWrap       = WrapMode::ClampToEdge;

		// Create image and store in the buffer
		Texture2D::Sptr image = std::make_shared<Texture2D>(descriptor);
		buffer.Resource = image;

		// Attach texture to the framebuffer
		glNamedFramebufferTexture(_rendererId, *attachment, image->GetHandle(), 0);
	}
}

bool Framebuffer::Validate() {
	// Get the framebuffer status, if it is not complete, log some errors
	GLenum result = glCheckNamedFramebufferStatus(_rendererId, GL_FRAMEBUFFER);
	if (result != GL_FRAMEBUFFER_COMPLETE) {
		switch (result) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				LOG_ERROR("Rendertarget failed to validate. One of the attachment points is framebuffer incomplete."); break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				LOG_ERROR("Rendertarget failed to validate. There are no attachments!"); break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				LOG_ERROR("Rendertarget failed to validate. Draw buffer is incomplete."); break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				LOG_ERROR("Rendertarget failed to validate. Read buffer is incomplete."); break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				LOG_ERROR("Rendertarget failed to validate. Check the formats of the attached targets"); break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				LOG_ERROR("Rendertarget failed to validate. Check the multisampling parameters on all attached targets"); break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				LOG_ERROR("Rendertarget failed to validate for unknown reason!"); break;
			case GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR:
				LOG_ERROR("Rendertarget failed to validate. Multiview targets issue!"); break;
			default: LOG_ERROR("Rendertarget failed to validate for unknown reason!");
		}
		_isValid = false;
		return false;
	} 
	else {
		_isValid = true;
		return true;
	}
}

bool Framebuffer::BindAttachment(RenderTargetAttachment attachment, int slot) const
{
	Texture2D::Sptr tex = GetTextureAttachment(attachment);
	if (tex != nullptr) {
		tex->Bind(slot);
		return true;
	}
	return false;
}

void Framebuffer::Bind(FramebufferBinding bindMode /*= FramebufferBinding::Draw*/) const {
	_currentBinding = bindMode;
	// Make sure that we're drawing to all the color buffers
	glNamedFramebufferDrawBuffers(_rendererId, _drawBuffers.size(), reinterpret_cast<const GLenum*>(_drawBuffers.data()));
	glBindFramebuffer(*bindMode, _rendererId);
}

void Framebuffer::Unbind() {
	// Only handle if we've been bound
	if (_currentBinding != FramebufferBinding::None) {
		// Unbind the framebuffer and clear our binding
		glBindFramebuffer(*_currentBinding, 0);
		_currentBinding = FramebufferBinding::None;
	}
}

void Framebuffer::Blit(const Sptr& source, const Sptr& dest, BufferFlags flags /*= BufferFlags::All*/, MagFilter filter /*= MagFilter::Linear*/) {
	// Bind this buffer as the read, and the unsampled as the write
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source ? source->GetHandle() : 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest ? dest->GetHandle() : 0);

	// Figure out bounds of the framebuffers
	glm::ivec4 srcBounds; 
	if (source != nullptr) {
		srcBounds ={ 0, 0, source->GetWidth(), source->GetHeight() };
	}
	else {
		int dimensions[4] ={ 0 };
		glGetIntegerv(GL_VIEWPORT, dimensions);
		srcBounds ={ dimensions[0], dimensions[1], dimensions[2], dimensions[3] };
	}
	glm::ivec4 dstBounds;
	if (dest != nullptr) {
		dstBounds ={ 0, 0, dest->GetWidth(), dest->GetHeight() };
	} 
	else {
		int dimensions[4] ={ 0 };
		glGetIntegerv(GL_VIEWPORT, dimensions);
		dstBounds ={ dimensions[0], dimensions[1], dimensions[2], dimensions[3] };
	}

	// Blit depth and stencil
	Blit(srcBounds, dstBounds, flags, filter);

	// Unbind both buffers
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Framebuffer::Blit(const glm::ivec4& srcBounds, const glm::ivec4& dstBounds, BufferFlags flags /*= BufferFlags::All*/, MagFilter filter /*= MagFilter::Linear*/) {
	if ((*(flags & BufferFlags::Depth) || *(flags & BufferFlags::Stencil)) && filter != MagFilter::Nearest) {
		static bool hasLogged = false;
		if (!hasLogged) {
			LOG_WARN("Attempting to blit depth and stencil using linear filtering, overriding. Subsequent warnings have been supressed");
			hasLogged = true;
		}
		filter = MagFilter::Nearest;
	}
	glBlitFramebuffer(
		srcBounds.x, srcBounds.y, srcBounds.z, srcBounds.w,
		dstBounds.x, dstBounds.y, dstBounds.z, dstBounds.w,
		*flags, *filter);
}

Framebuffer::Sptr Framebuffer::Clone() {
	return std::make_shared<Framebuffer>(_description);
}

void Framebuffer::SetDebugName(const std::string& name) {
	IGraphicsResource::SetDebugName(name);

	// We'll also update the name for all our children
	for (const auto& attachment : _targets) {
		static char buffer[256];
		sprintf_s(buffer, 256, "%s_%s", name.c_str(), (~attachment.first).c_str());
		attachment.second.Resource->SetDebugName(buffer);
	}
}

GlResourceType Framebuffer::GetResourceClass() const {
	return GlResourceType::FrameBuffer;
}

nlohmann::json Framebuffer::ToJson() const {
	nlohmann::json result ={
		{ "width", _description.Width },
		{ "height", _description.Height },
		{ "attachments", nlohmann::json() }
	};

	// Iterate over our attachments and serialize them
	for (const auto& kvp : _targets) {
		// We'll create a separate JSON object for the RenderTargetDescriptor
		nlohmann::json attachmentInfo = nlohmann::json();
		attachmentInfo["use-texture"] = kvp.second.Description.UseTexture;
		attachmentInfo["format"] = ~kvp.second.Description.Format;

		// Store attachments keyed on attachment point
		result["attachments"][~kvp.first] = attachmentInfo;
	}

	return result;
}

Framebuffer::Sptr Framebuffer::FromJson(const nlohmann::json& blob) {
	FramebufferDescriptor result = FramebufferDescriptor();
	result.Width  = JsonGet(blob, "width", 0);
	result.Height = JsonGet(blob, "height", 0);

	if (blob.contains("attachments") && blob["attachments"].is_object()) {
		// Iterate over all objects
		for (auto& [key, value] : blob["attachments"].items()) {
			// Try parsing attachment slot from the key
			RenderTargetAttachment attachment = ParseRenderTargetAttachment(key, RenderTargetAttachment::Unknown);

			// Parse descriptor from the value
			RenderTargetDescriptor descriptor = RenderTargetDescriptor();
			descriptor.UseTexture = JsonGet(value, "use-texture", true);
			descriptor.Format = JsonParseEnum(RenderTargetType, value, "format", RenderTargetType::Unknown);

			// If valid, add it, otherwise skip
			if (descriptor.Format != RenderTargetType::Unknown && attachment != RenderTargetAttachment::Unknown) {
				result.RenderTargets[attachment] = descriptor;
			}
			else {
				LOG_WARN("Failed to load render target attachment type \"{}\"", ~attachment);
			}
		}
	}

	return std::make_shared<Framebuffer>(result);
}

Framebuffer::RenderTarget::RenderTarget() :
	Resource(nullptr),
	IsRenderBuffer(false),
	Description(RenderTargetDescriptor())
{ }
