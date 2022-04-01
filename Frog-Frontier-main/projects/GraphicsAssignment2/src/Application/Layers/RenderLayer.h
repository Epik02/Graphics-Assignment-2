#pragma once
#include "../ApplicationLayer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Buffers/UniformBuffer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/VertexArrayObject.h"

#define MAX_LIGHTS 8

ENUM_FLAGS(RenderFlags, uint32_t,
	None = 0,
	EnableColorCorrection = 1 << 0
);

class RenderLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(RenderLayer); 

	// Structure for our frame-level uniforms, matches layout from
	// fragments/frame_uniforms.glsl
	// For use with a UBO.
	struct FrameLevelUniforms {
		// The camera's view matrix
		glm::mat4 u_View;
		// The camera's projection matrix
		glm::mat4 u_Projection;
		// The combined viewProject matrix
		glm::mat4 u_ViewProjection;
		// The camera's position in world space
		glm::vec4 u_CameraPos;
		// The time in seconds since the start of the application
		float u_Time;
		// The time in seconds since the previous frame
		float u_DeltaTime;
		// Bitfield representing up to 32 bool values to enable/disable stuff
		RenderFlags u_RenderFlags;
		float u_ZNear;
		float u_ZFar;
	};

	// Structure for our instance-level uniforms, matches layout from
	// fragments/frame_uniforms.glsl
	// For use with a UBO.
	struct InstanceLevelUniforms {
		// Complete MVP
		glm::mat4 u_ModelViewProjection;
		// Just the model transform, we'll do worldspace lighting
		glm::mat4 u_Model;
		// To go from model space to view space
		glm::mat4 u_ModelView;
		// Normal Matrix for transforming normals
		glm::mat4 u_NormalMatrix;
	};

	/// <summary>
	/// Represents a c++ struct layout that matches that of
	/// our multiple light uniform buffer
	/// 
	/// Note that we have to do some weirdness since OpenGl has a
	/// thing for packing structures to sizeof(vec4)
	/// </summary>
	struct LightingUboStruct {
		struct Light {
			glm::vec3 Position;
			float Intensity;
			// Since these are tightly packed, will match the vec4 in light
			glm::vec3 Color;
			float     Attenuation;
		};

		// Since these are tightly packed, will match the vec4 in the UBO
		glm::vec3 AmbientCol;
		float     NumLights;

		Light     Lights[MAX_LIGHTS];
		// NOTE: our shaders expect a mat3, but due to the STD140 layout, each column of the
		// vec3 needs to be padded to the size of a vec4, hence the use of a mat4 here
		glm::mat4 EnvironmentRotation;
	};

	RenderLayer();
	virtual ~RenderLayer();

	/// <summary>
	/// Gets the primary framebuffer that is being rendered to
	/// </summary>
	const Framebuffer::Sptr& GetPrimaryFBO() const;

	bool IsBlitEnabled() const;
	void SetBlitEnabled(bool value);

	const glm::vec4& GetClearColor() const;
	void SetClearColor(const glm::vec4& value);

	void SetRenderFlags(RenderFlags value);
	RenderFlags GetRenderFlags() const;

	const Framebuffer::Sptr& GetLightingBuffer() const;
	const Framebuffer::Sptr& GetRenderOutput() const;
	const Framebuffer::Sptr& GetGBuffer() const;

	// Inherited from ApplicationLayer

	virtual void OnAppLoad(const nlohmann::json& config) override;
	virtual void OnPreRender() override;
	virtual void OnRender(const Framebuffer::Sptr& prevLayer) override;
	virtual void OnPostRender() override;
	virtual void OnWindowResize(const glm::ivec2& oldSize, const glm::ivec2& newSize) override;

protected:
	Framebuffer::Sptr   _primaryFBO;
	Framebuffer::Sptr   _lightingFBO;
	Framebuffer::Sptr   _outputBuffer;

	ShaderProgram::Sptr _clearShader;
	ShaderProgram::Sptr _lightAccumulationShader;
	ShaderProgram::Sptr _compositingShader;
	ShaderProgram::Sptr _shadowShader;

	VertexArrayObject::Sptr _fullscreenQuad;

	bool              _blitFbo;
	glm::vec4         _clearColor;
	RenderFlags       _renderFlags;

	const int FRAME_UBO_BINDING = 0;
	UniformBuffer<FrameLevelUniforms>::Sptr _frameUniforms;

	const int INSTANCE_UBO_BINDING = 1;
	UniformBuffer<InstanceLevelUniforms>::Sptr _instanceUniforms;

	const int LIGHTING_UBO_BINDING = 2;
	UniformBuffer<LightingUboStruct>::Sptr _lightingUbo;

	void _InitFrameUniforms();
	void _RenderScene(const glm::mat4& view, const glm::mat4&Projection);

	void _AccumulateLighting();
	void _Composite();
	void _ClearFramebuffer(Framebuffer::Sptr& buffer, const glm::vec4* colors, int layers);
};