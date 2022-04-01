#pragma once
#include "Gameplay/Components/IComponent.h"

/// <summary>
/// A rect transform is used by 2D GUI components for positioning within
/// screen space
/// </summary>
class RectTransform : public Gameplay::IComponent {

public:
	typedef std::shared_ptr<RectTransform> Sptr;
	typedef std::weak_ptr<RectTransform> Wptr;

	RectTransform();
	virtual ~RectTransform();

	/// <summary>
	/// Gets the center of this GUI object in screen space coords
	/// </summary>
	const glm::vec2& GetPosition() const;
	/// <summary>
	/// Sets the center position of this GUI object in screen space coords
	/// </summary>
	void SetPosition(const glm::vec2& pos);

	/// <summary>
	/// Gets the minimum bounds of this GUI object in screen space coords
	/// </summary>
	glm::vec2 GetMin() const;
	/// <summary>
	/// Sets the minimum bounds of this GUI object in screen space coords
	/// </summary>
	void SetMin(const glm::vec2& value);

	/// <summary>
	/// Gets the maximum bounds of this GUI object in screen space coords
	/// </summary>
	glm::vec2 GetMax() const;
	/// <summary>
	/// Sets the maximum bounds of this GUI object in screen space coords
	/// </summary>
	void SetMax(const glm::vec2& value);

	/// <summary>
	/// Gets the size of this GUI object in screen space coords
	/// </summary>
	glm::vec2 GetSize() const;
	/// <summary>
	/// Sets the size of this GUI object in screen space coords,
	/// note that it will grown from it's center position
	/// </summary>
	void SetSize(const glm::vec2& value);

	/// <summary>
	/// Sets the rotation of the element in degrees,
	/// note that this rotation is around the center of the bounds
	/// </summary>
	void SetRotationDeg(float value);
	/// <summary>
	/// Gets the rotation around the center in degrees
	/// </summary>
	float GetRotationDeg() const;

	/// <summary>
	/// Gets a transform that will transform (0,0) into the top left corner
	/// of this transform
	/// </summary>
	const glm::mat3& GetLocalTransform() const;

public:
	// Inherited from IComponent

	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	virtual void StartGUI() override;
	virtual void FinishGUI() override;
	static RectTransform::Sptr FromJson(const nlohmann::json& blob);
	MAKE_TYPENAME(RectTransform);

protected:
	glm::vec2 _pivot;
	glm::vec2 _position;
	glm::vec2 _halfSize;
	float     _rotation;

	mutable glm::mat3 _transform;
	mutable bool _transformDirty;

	void __RecalcTransforms() const;
};