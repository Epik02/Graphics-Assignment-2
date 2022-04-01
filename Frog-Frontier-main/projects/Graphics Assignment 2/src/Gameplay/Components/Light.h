#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

// TODO: Support lights other than point lights in our shaders
ENUM(LightType, int,
	Unknown,
	Point,
	Directional,
	Cone
);

/// <summary>
/// A structure to store our light info
/// </summary>
class Light : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<Light> Sptr;

	std::weak_ptr<Gameplay::IComponent> Panel;

	Light();
	virtual ~Light();

	const glm::vec3& GetColor() const;
	void SetColor(const glm::vec3& value);

	const glm::vec3& GetDirection() const;
	void SetDirection(const glm::vec3& value);

	const glm::vec3& GetParams() const;
	void SetParams(const glm::vec3& value);

	float GetIntensity() const;
	void SetIntensity(float value);

	float GetRadius() const;
	void SetRadius(float value);

	LightType GetType() const;
	void SetType(LightType value);

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(Light);
	virtual nlohmann::json ToJson() const override;
	static Light::Sptr FromJson(const nlohmann::json& blob);

protected:
	LightType _type;
	glm::vec3 _color;
	glm::vec3 _direction;
	glm::vec3 _params;
	float     _radius;
	float     _intensity;
};