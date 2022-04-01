#pragma once
#include "../ApplicationLayer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Buffers/UniformBuffer.h"

class InterfaceLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(InterfaceLayer);

	InterfaceLayer();
	virtual ~InterfaceLayer();
		
	// Inherited from ApplicationLayer

	virtual void OnRender(const Framebuffer::Sptr& prevLayer) override;
	virtual void OnWindowResize(const glm::ivec2& oldSize, const glm::ivec2& newSize) override;
};