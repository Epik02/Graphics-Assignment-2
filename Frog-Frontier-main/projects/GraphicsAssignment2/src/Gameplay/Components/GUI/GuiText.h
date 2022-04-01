#pragma once
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Graphics/Font.h"

/// <summary>
/// Renders text for UI components
/// </summary>
class GuiText : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<GuiText> Sptr;
	typedef std::weak_ptr<GuiText> Wptr;

	GuiText();
	virtual ~GuiText();

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
	/// Gets the ASCII string being rendered
	/// </summary>
	std::string GetText() const;
	/// <summary>
	/// Sets the ASCII text being rendered
	/// </summary>
	void SetText(const std::string& value);

	/// <summary>
	/// Gets the Unicode string being rendered
	/// </summary>
	const std::wstring& GetTextUnicode() const;
	/// <summary>
	/// Sets the Unicode text being rendered
	/// </summary>
	void SetTextUnicode(const std::wstring& value);


	/// <summary>
	/// Gets the scaling applied to the text for this element, 
	/// as a multiple of the font size
	/// </summary>
	const float GetTextScale() const;
	/// <summary>
	/// Sets the scaling applied to the text for this element, 
	/// as a multiple of the font size
	/// </summary>
	void SetTextScale(float value);

	/// <summary>
	/// Gets the font used to render this text
	/// </summary>
	const Font::Sptr& GetFont() const;
	/// <summary>
	/// Sets the font to use for this text
	/// </summary>
	void SetFont(const Font::Sptr& font);


public:
	// Inherited from IComponent

	virtual void Awake() override;
	virtual void RenderGUI() override;
	virtual void RenderImGui() override;
	MAKE_TYPENAME(GuiText);
	virtual nlohmann::json ToJson() const override;
	static GuiText::Sptr FromJson(const nlohmann::json& blob);

protected:
	std::wstring    _text;
	glm::vec4       _color;
	Font::Sptr      _font;
	glm::vec2       _textSize;
	float           _textScale;

	RectTransform::Sptr _transform;
};