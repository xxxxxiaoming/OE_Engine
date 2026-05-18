#include "RenderExperments.h"

int main()
{
	Engine::Renderer renderer{ 1280, 720, "Hello OpenGL!" };

	renderer.EnableBlend();
	renderer.EnableDepthTest();

	//StencilTestExperiment(std::string{ "res/assets/backpack/backpack.obj" }, renderer);
	// RenderTargetExperiment(renderer);
	// DrawASimpleHouseUsingGS(renderer);
	InstanceExperiment(renderer);
	renderer.Clear();
	return 0;
}