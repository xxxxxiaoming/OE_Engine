#include <string>
#include <iostream>
#include <vector>

#include "RenderExperments.h"

int main()
{
	Engine::Renderer renderer{ 1280, 720, "Hello OpenGL!" };

	renderer.EnableBlend();
	renderer.EnableDepthTest();

	LoadModelExperiment(std::string{ "res/assets/backpack/backpack.obj" }, renderer);

	renderer.Clear();
	return 0;
}