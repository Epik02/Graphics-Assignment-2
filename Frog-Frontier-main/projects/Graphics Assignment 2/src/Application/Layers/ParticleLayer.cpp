#include "ParticleLayer.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Application/Application.h"
#include "RenderLayer.h"

ParticleLayer::ParticleLayer() :
	ApplicationLayer()
{
	Name = "Particles";
	Overrides = AppLayerFunctions::OnUpdate | AppLayerFunctions::OnPostRender;
}

ParticleLayer::~ParticleLayer()
{ }

void ParticleLayer::OnUpdate()
{
	Application& app = Application::Get();

	// Only update the particle systems when the game is playing, so we can edit them in
	// the inspector
	if (app.CurrentScene()->IsPlaying) {
		app.CurrentScene()->Components().Each<ParticleSystem>([](const ParticleSystem::Sptr& system) {
			if (system->IsEnabled) {
				system->Update();
			}
		});
	}
}

void ParticleLayer::OnPostRender()
{
	Application& app = Application::Get();
	const glm::uvec4& viewport = app.GetPrimaryViewport();

	// Restore viewport to game viewport

	RenderLayer::Sptr renderer = app.GetLayer<RenderLayer>();
	const Framebuffer::Sptr renderOutput = renderer->GetRenderOutput();
	renderOutput->Bind();
	glViewport(0, 0, renderOutput->GetWidth(), renderOutput->GetHeight());

	Application::Get().CurrentScene()->Components().Each<ParticleSystem>([](const ParticleSystem::Sptr& system) {
		if (system->IsEnabled) {
			system->Render(); 
		}
	});

	renderer->GetRenderOutput()->Unbind();
}
