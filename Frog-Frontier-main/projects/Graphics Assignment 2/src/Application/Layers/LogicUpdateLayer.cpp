#include "LogicUpdateLayer.h"
#include "../Application.h"
#include "../Timing.h"

LogicUpdateLayer::LogicUpdateLayer() :
	ApplicationLayer()
{
	Name = "Logic";
	Overrides = AppLayerFunctions::OnUpdate;
}

LogicUpdateLayer::~LogicUpdateLayer() = default;

void LogicUpdateLayer::OnUpdate()
{
	Application& app = Application::Get();

	// Perform updates for all components
	app.CurrentScene()->Update(Timing::Current().DeltaTime());

	// Update our worlds physics!
	app.CurrentScene()->DoPhysics(Timing::Current().DeltaTime());
}
