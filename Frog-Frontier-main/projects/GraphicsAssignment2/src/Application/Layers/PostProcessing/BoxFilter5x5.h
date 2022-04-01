#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Framebuffer.h"

class BoxFilter5x5 : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(BoxFilter5x5);
	float Filter[25];

	BoxFilter5x5();
	virtual ~BoxFilter5x5();

	virtual void Apply(const Framebuffer::Sptr& gBuffer) override;
	virtual void RenderImGui() override;

	// Inherited from IResource

	BoxFilter5x5::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
};