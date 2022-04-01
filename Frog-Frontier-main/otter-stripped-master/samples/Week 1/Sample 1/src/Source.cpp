/*
Week 0 tutorial sample - setting up OTTER
Quinn Daggett 2021
*/

#include "NOU/App.h"
#include "NOU/Input.h"
#include "NOU/Entity.h"
#include "NOU/CCamera.h"
#include "NOU/CMeshRenderer.h"
#include "NOU/Shader.h"
#include "NOU/GLTFLoader.h"

#include "Logging.h"

#include <memory>

using namespace nou;

int main() {
	// Initialization
	App::Init("Week 0 tutorial - OTTER setup", 800, 600);
	App::SetClearColor(glm::vec4(0.0f, 0.27f, 0.40f, 1.0f)); // Reminder: SetClearColor uses RGB values of 0-1 instead of 0-255

	App::Tick();

	// Update loop
	while (!App::IsClosing() && !Input::GetKeyDown(GLFW_KEY_ESCAPE))
	{
		App::FrameStart();

		App::SwapBuffers();
	}

	App::Cleanup();

	return 0;
}