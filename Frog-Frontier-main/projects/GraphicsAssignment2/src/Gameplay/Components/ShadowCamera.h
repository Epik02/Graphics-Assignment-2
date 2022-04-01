#pragma once
#include "Graphics/Framebuffer.h"
#include "Graphics/Textures/Texture2D.h"
#include "Gameplay/Components/IComponent.h"
#include "Graphics/ShaderProgram.h"

ENUM_FLAGS(ShadowFlags, uint32_t,
	None = 0,
	ProjectionEnabled  = 1 << 0,
	PcfEnabled         = 1 << 1,
	AttenuationEnabled = 1 << 2,
	WidePcfEnabled     = 1 << 3
);

/**
 * A camera with a depth buffer that lets us render shadows like a camera
 * Also contains color and projector mask info
 */
class ShadowCamera final : public Gameplay::IComponent {
public:
	MAKE_PTRS(ShadowCamera);

	/// <summary>
	/// Stores some bit flags for toggling shadow functionality in the shader
	/// </summary>
	ShadowFlags Flags;

	float Bias;
	float NormalBias;
	float Intensity;
	float Range;

	ShadowCamera();
	virtual ~ShadowCamera();

	/// <summary>
	/// Sets the color for the light. When projecting, this is multiplied
	/// by the color from the projection mask
	/// </summary>
	/// <param name="value">The color of the light</param>
	void SetColor(const glm::vec4& value);
	/// <summary>
	/// Gets the color of this light
	/// </summary>
	const glm::vec4& GetColor() const;

	/// <summary>
	/// Resizes this light's depth buffer, both dimensions must be non-zero
	/// </summary>
	/// <param name="value">The new size of the buffer, in pixels</param>
	void SetBufferResolution(const glm::ivec2& value);
	/// <summary>
	/// Returns the resolution of this light's depth buffer in pixels
	/// </summary>
	const glm::ivec2& GetBufferResolution() const;

	/// <summary>
	/// Overrides the projection matrix for this light
	/// </summary>
	/// <param name="value">The new value for the projection matrix</param>
	void SetProjection(const glm::mat4& value);
	/// <summary>
	/// Gets the light's projection matrix. Does NOT need to be perspective
	/// </summary>
	const glm::mat4& GetProjection() const;

	/// <summary>
	/// Returns this light's view projection
	/// </summary>
	glm::mat4 GetViewProjection() const;

	/// <summary>
	/// Sets the image to use for projection, and enables image projection
	/// </summary>
	/// <param name="image">The image to project from this light</param>
	void SetProjectionMask(const Texture2D::Sptr& image);
	/// <summary>
	/// Gets the image used for projection with this light
	/// </summary>
	const Texture2D::Sptr& GetProjectionMask() const;

	/// <summary>
	/// Gets the shadow camera's depth buffer that it renders to
	/// </summary>
	const Framebuffer::Sptr& GetDepthBuffer() const;

	// Inherited from IComponent

	virtual void OnLoad();
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static ShadowCamera::Sptr FromJson(const nlohmann::json& data);
	MAKE_TYPENAME(ShadowCamera);

protected:
	// Framebuffer we render into to get depth
	Framebuffer::Sptr _depthBuffer;
	// The image to project from this light
	Texture2D::Sptr   _projectionMask;
	// The color of the light
	glm::vec4         _color;
	// The resolution of _depthBuffer in pixels
	glm::ivec2        _bufferResolution;
	// The projection matrix of the light
	glm::mat4         _projectionMatrix;
};