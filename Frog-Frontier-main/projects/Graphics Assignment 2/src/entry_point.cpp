#define GLM_SWIZZLE 
#include "Application/Application.h"

int main(int argc, char** args) {
	Logger::Init();

	// TODO: parse arguments?

	Application::Start(argc, args);

	Logger::Uninitialize();
}