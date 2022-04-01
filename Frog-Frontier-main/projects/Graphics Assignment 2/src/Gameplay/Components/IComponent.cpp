#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

namespace Gameplay {
	GameObject* IComponent::GetGameObject() const {
		return _context;
	}

	std::weak_ptr<IComponent>& IComponent::SelfRef() {
		return _weakSelfPtr;
	}

	void IComponent::LoadBaseJson(const Sptr& result, const nlohmann::json& blob)
	{
		result->OverrideGUID(Guid(blob["guid"]));
		result->IsEnabled = blob["enabled"];
	}

	void IComponent::SaveBaseJson(const Sptr& instance, nlohmann::json& data)
	{
		data["guid"] = instance->GetGUID().str();
		data["enabled"] = instance->IsEnabled;
	}

	IComponent::IComponent() :
		IResource(),
		IsEnabled(true),
		_realType(typeid(IComponent)),
		_context(nullptr)
	{ }

	IComponent::~IComponent() {
		_context->GetScene()->Components().Remove(this);
	}
}
