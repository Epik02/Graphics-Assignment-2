#pragma once

#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Graphics/Textures/Texture2D.h"

/// <summary>
/// Draws a textured background for UI components
/// </summary>
class GuiPanel : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<GuiPanel> Sptr;
	typedef std::weak_ptr<GuiPanel> Wptr;

	GuiPanel();
	virtual ~GuiPanel();

	/// <summary>
	/// Sets the color multiplier for this GUI object
	/// </summary>
	/// <param name="color"></param>
	void SetColor(const glm::vec4& color);
	/// <summary>
	/// Gets the color multiplier for this GUI object
	/// </summary>
	const glm::vec4& GetColor() const;

	/// <summary>
	/// Gets the border radius for this GUI object in pixels
	/// </summary>
	int GetBorderRadius() const;
	/// <summary>
	/// Sets the border radius of this GUI object in pixels, this should be the
	/// distance from the edge of the texture to make slices for rounded rectangles
	/// </summary>
	void SetBorderRadius(int value);

	/// <summary>
	/// Gets the background texture for this panel
	/// </summary>
	Texture2D::Sptr GetTexture() const;
	/// <summary>
	/// Sets the background texture to use for this panel
	/// </summary>
	void SetTexture(const Texture2D::Sptr& value);

public:
	virtual void Awake() override;
	virtual void StartGUI() override;
	virtual void FinishGUI() override;
	virtual void RenderImGui() override;
	MAKE_TYPENAME(GuiPanel);
	virtual nlohmann::json ToJson() const override;
	static GuiPanel::Sptr FromJson(const nlohmann::json& blob);

protected:
	int             _borderRadius;
	Texture2D::Sptr _texture;
	glm::vec4       _color;

	RectTransform::Sptr _transform;
};