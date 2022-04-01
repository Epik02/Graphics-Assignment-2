#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Framebuffer.h"

class BoxFilter3x3 : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(BoxFilter3x3);
	float Filter[9];

	BoxFilter3x3();
	virtual ~BoxFilter3x3();

	virtual void Apply(const Framebuffer::Sptr& gBuffer) override;
	virtual void RenderImGui() override;

	// Inherited from IResource

	BoxFilter3x3::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
};

