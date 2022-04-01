#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture3D.h"

class OutlineEffect : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(OutlineEffect);

	OutlineEffect();
	virtual ~OutlineEffect();

	virtual void Apply(const Framebuffer::Sptr& gBuffer) override;
	virtual void RenderImGui() override;

	// Inherited from IResource

	OutlineEffect::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
	glm::vec4           _outlineColor;
	float               _scale;
	float               _depthThreshold;
	float               _normalThreshold;
	float               _depthNormalThreshold;
	float               _depthNormalThresholdScale;

};


