#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"

MaterialSwapBehaviour::MaterialSwapBehaviour() :
	IComponent(),
	_renderer(nullptr),
	EnterMaterial(nullptr),
	ExitMaterial(nullptr)
{ }
MaterialSwapBehaviour::~MaterialSwapBehaviour() = default;

void MaterialSwapBehaviour::OnEnteredTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && EnterMaterial) {
		_renderer->SetMaterial(EnterMaterial);
	}
	LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
}

void MaterialSwapBehaviour::OnLeavingTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && ExitMaterial) {
		_renderer->SetMaterial(ExitMaterial);
	}
	LOG_INFO("Left trigger: {}", trigger->GetGameObject()->Name);
}

void MaterialSwapBehaviour::Awake() {
	_renderer = GetComponent<RenderComponent>();
}

void MaterialSwapBehaviour::RenderImGui() { }

nlohmann::json MaterialSwapBehaviour::ToJson() const {
	return {
		{ "enter_material", EnterMaterial != nullptr ? EnterMaterial->GetGUID().str() : "null" },
		{ "exit_material", ExitMaterial != nullptr ? ExitMaterial->GetGUID().str() : "null" }
	};
}

MaterialSwapBehaviour::Sptr MaterialSwapBehaviour::FromJson(const nlohmann::json& blob) {
	MaterialSwapBehaviour::Sptr result = std::make_shared<MaterialSwapBehaviour>();
	result->EnterMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["enter_material"]));
	result->ExitMaterial  = ResourceManager::Get<Gameplay::Material>(Guid(blob["exit_material"]));
	return result;
}
