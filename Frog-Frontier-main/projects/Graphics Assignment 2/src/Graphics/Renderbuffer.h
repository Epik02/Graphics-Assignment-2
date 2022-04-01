#pragma once
#include "Graphics/IGraphicsResource.h"
#include "Graphics/GlEnums.h"

/**
 * Describes the parameters for creating an OpenGL render buffer
 */
struct RenderbufferDescription {
	/**
	 * The width of the render buffer in pixels
	 */
	uint32_t         Width;
	/**
	 * The height of the render buffer in pixels
	 */
	uint32_t         Height;
	/**
	 * The multisample count, if this render buffer is multisampled
	 */
	uint8_t          MultisampleCount;
	/**
	 * The internal format of this render buffer
	 */
	RenderTargetType Format;

	RenderbufferDescription() :
		Width(0),
		Height(0),
		MultisampleCount(1),
		Format(RenderTargetType::ColorRgba8) 
	{ }
};

class Renderbuffer final : public IGraphicsResource {
public:
	DEFINE_RESOURCE(Renderbuffer);

	Renderbuffer(const RenderbufferDescription& description);
	~Renderbuffer();

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	RenderTargetType GetFormat() const;

	// Inherited from IGraphicsResource

	virtual GlResourceType GetResourceClass() const override;

protected:
	RenderbufferDescription _description;
};