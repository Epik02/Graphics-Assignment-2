#include "Graphics/Renderbuffer.h"

Renderbuffer::Renderbuffer(const RenderbufferDescription& description) :
	IGraphicsResource(),
	_description(description)
{
	glCreateRenderbuffers(1, &_rendererId);

	if (_description.MultisampleCount > 1) {
		glNamedRenderbufferStorageMultisample(_rendererId, _description.MultisampleCount, *_description.Format, _description.Width, _description.Height);
	}
	else {
		glNamedRenderbufferStorage(_rendererId, *_description.Format, _description.Width, _description.Height);
	}
}

Renderbuffer::~Renderbuffer() {
	glDeleteRenderbuffers(1, &_rendererId);
}

uint32_t Renderbuffer::GetWidth() const {
	return _description.Width;
}

uint32_t Renderbuffer::GetHeight() const {
	return _description.Height;
}

RenderTargetType Renderbuffer::GetFormat() const {
	return _description.Format;
}

GlResourceType Renderbuffer::GetResourceClass() const {
	return GlResourceType::RenderBuffer;
}