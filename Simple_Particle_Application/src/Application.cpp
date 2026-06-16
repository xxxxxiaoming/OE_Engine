#include "RenderExperments.h"

int main()
{
	Engine::Renderer renderer{ 1920, 1080, "Hello OpenGL!" };

	renderer.EnableBlend();
	renderer.EnableDepthTest();

	// StencilTestExperiment(std::string{ "res/assets/backpack/backpack.obj" }, renderer);
	// RenderTargetExperiment(renderer);
	// DrawASimpleHouseUsingGS(renderer);
	// InstanceExperiment(renderer);
	//AdvancedLighting(renderer);
	PBRLighting(renderer);
	renderer.Clear();
	return 0;
}