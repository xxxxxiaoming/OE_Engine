#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #include <complex>

#include "Engine.h"

// #define TRIANGLE_BUFFER_SIZE (sizeof(float) * 6)
// #define RECTANGLE_BUFFER_SIZE (sizeof(float) * 16)

int main()
{
	Engine::Renderer renderer{ 1280, 720, "Hello OpenGL!" };
	
	renderer.EnableBlend();
	renderer.EnableDepthTest();

	/* Camera and controller */
	glm::vec3 camPostion = glm::vec3(0.0f, 0.0f, 0.0f);
	Engine::Camera cam{camPostion, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)};
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 40.0f, 10.0f};

	/* Bind keys */
	camController.BindKey(Engine::Operations::MoveUp, GLFW_KEY_Q);
	camController.BindKey(Engine::Operations::MoveDown, GLFW_KEY_E);
	camController.BindKey(Engine::Operations::MoveLeft, GLFW_KEY_A);
	camController.BindKey(Engine::Operations::MoveRight, GLFW_KEY_D);
	camController.BindKey(Engine::Operations::MoveForward, GLFW_KEY_W);
	camController.BindKey(Engine::Operations::MoveBackward, GLFW_KEY_S);
	camController.BindKey(Engine::Operations::LookUp, GLFW_KEY_UP);
	camController.BindKey(Engine::Operations::LookDown, GLFW_KEY_DOWN);
	camController.BindKey(Engine::Operations::LookLeft, GLFW_KEY_LEFT);
	camController.BindKey(Engine::Operations::LookRight, GLFW_KEY_RIGHT);

	/* Possess camera */
	camController.PossessCamera(&cam);
	
	/* Cube */
	Engine::Vertex cubeVertices[48];
	Engine::vec3 centers[2] = {
		Engine::vec3{0.0f, 0.0f, 0.0f},
		Engine::vec3{100.0f, 0.0f, -270.0f}
	};
	
	/* Shader */
	std::string vsFilePath = "res/shader/VSShader.vert";
	std::string fsFilePath = "res/shader/PhongLightShader.frag";

	/* Texture */
	std::string textures[6] = {
		std::string{"res/texture/AO_Logo_Square.png"},
		std::string{"res/texture/Roland-Garros-logo.png"},
		std::string{"res/texture/US_OPEN_Logo.png"},
		std::string{"res/texture/wimbledon-logo.png"},
		std::string{"res/texture/container2.png"},
		std::string{"res/texture/container2_specular.png"},
	};
	const int textureSlots[6] = { 0, 1, 2, 3, 4, 5 };

	/* Material */
	Engine::Material cubeMaterial{ vsFilePath, fsFilePath, textures, textureSlots, 6 };
	
	/* Size */
	float widths[2] {70.0f, 70.0f};
	float heights[2] {70.0f, 70.0f};
	float depths[2] {70.0f, 70.0f};
	int cubeIndices[72];
	float textureSlot = 0.0f;
	const float cubeRotateSpeed = 0.0f;
	
	Engine::CreateCube<2>(centers, widths, heights, depths, cubeVertices, cubeIndices);
	for(auto& vertex : cubeVertices)
		vertex.textureSlot = textureSlot;

	Engine::Object cube{ cubeVertices, cubeIndices, 48, 72};
	cube.BindMaterial(&cubeMaterial);

	/* Light */
	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);
	Engine::Vertex lightVertices[24];
	Engine::vec3 lightCenters[1] = {
		{lightPosition.x, lightPosition.y, lightPosition.z},
	};
	
	/* Shader */
	std::string lightVSFilePath = "res/shader/VSShaderLight.vert";
	std::string lightFSFilePath = "res/shader/FSShaderLight.frag";

	/* Material */
	Engine::Material lightMaterial{ lightVSFilePath, lightFSFilePath, nullptr, nullptr, 0 };
	
	/* Size */
	float widthsL[1]{ 20.0f };
	float heightsL[1]{ 20.0f };
	float depthsL[1]{ 20.0f };
	float lightColor[3]{ 1.0f, 1.0f, 1.0f };

	Engine::CreateCube<1>(lightCenters, widthsL, heightsL, depthsL, lightVertices, cubeIndices);

	for (auto& vertex : lightVertices)
	{
		vertex.color = { lightColor[0], lightColor[1], lightColor[2], 1.0f };
	}

	Engine::Object light{ lightVertices, cubeIndices, 24, 36};
	light.BindMaterial(&lightMaterial);
	
	glm::mat4 modelMatLight = glm::translate(glm::mat4(1.0f), glm::vec3(70, 40, 0));
	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -170));
	glm::mat4 viewMat = cam.GetViewMatrix();
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 640.0f/360.0f, 0.1f, 3000.0f);
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	glm::vec3 lightWorldPos = modelMatLight * glm::vec4(lightPosition, 1.0f);

	Engine::Shader& shader = cubeMaterial.GetShader();
	shader.Use();
	
	shader.SetUniform1i("u_Material.diffuse", 4);
	shader.SetUniform1i("u_Material.specular", 5);
	shader.SetUniform1i("u_Material.shininess", 64);

	/* 配置光照 */
	shader.SetUniform1i("u_LightConfig.enableDirectionLight", 1);
	shader.SetUniform1i("u_LightConfig.pointLightNum", 1);
	shader.SetUniform1i("u_LightConfig.spotLightNum", 1);

	/* 方向光配置 */
	shader.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
	shader.SetUniform3f("u_DirectionLight.color.ambient", 0.1f, 0.1f, 0.1f);
	shader.SetUniform3f("u_DirectionLight.color.diffuse", 1.0f, 1.0f, 1.0f);
	shader.SetUniform3f("u_DirectionLight.color.specular", 0.5f, 0.5f, 0.5f);

	/* 点光源配置 */
	shader.SetUniform3f("u_PointLights[0].position", lightWorldPos.x, lightWorldPos.y, lightWorldPos.z);
	shader.SetUniform1f("u_PointLights[0].constant", 1.0f);
	shader.SetUniform1f("u_PointLights[0].linear", 0.001f);
	shader.SetUniform1f("u_PointLights[0].quadratic", 0.0001f);
	shader.SetUniform3f("u_PointLights[0].color.ambient", 0.1f, 0.1f, 0.1f);
	shader.SetUniform3f("u_PointLights[0].color.diffuse", 0.5f, 0.5f, 0.5f);
	shader.SetUniform3f("u_PointLights[0].color.specular", 1.0f, 1.0f, 1.0f);

	/* 聚光源配置 */
	shader.SetUniform1f("u_SpotLights[0].innerAngle", glm::radians(5.0f));
	shader.SetUniform1f("u_SpotLights[0].outterAngle", glm::radians(10.0f));
	shader.SetUniform1f("u_SpotLights[0].constant", 1.0f);
	shader.SetUniform1f("u_SpotLights[0].linear", 0.001f);
	shader.SetUniform1f("u_SpotLights[0].quadratic", 0.0001f);
	shader.SetUniform3f("u_SpotLights[0].color.ambient", 0.1f, 0.1f, 0.1f);
	shader.SetUniform3f("u_SpotLights[0].color.diffuse", 1.0f, 1.0f, 1.0f);
	shader.SetUniform3f("u_SpotLights[0].color.specular", 1.0f, 1.0f, 1.0f);

	shader.UnUse();
	
	double lastFrame = glfwGetTime();	
	while (!renderer.CheckWindowShouldClose())
	{
		double deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		/* Keyboard input */
		camController.ProcessInput(static_cast<float>(deltaTime));

		/* Transform matrix */
		modelMat = glm::rotate(modelMat, glm::radians(static_cast<float>(deltaTime) * cubeRotateSpeed), glm::vec3{ 0, 1.0f, 0 });
		glm::mat4 viewMat = cam.GetViewMatrix();
		normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));

		renderer.OnRender();
		
		glm::vec3 camPosRealTime = cam.GetPosition();
		glm::vec3 camFrontRealTime = cam.GetFront();

		cube.OnDraw();
		
		shader.SetUniformMatrix4f("u_Model", modelMat);
		shader.SetUniformMatrix4f("u_View", viewMat);
		shader.SetUniformMatrix4f("u_Projection", projectionMat);
		shader.SetUniformMatrix3f("u_NormalMat", normalMat);
		shader.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);
		
		/* 每帧更新聚光源的位置，模拟控制手电筒^-^ */
		shader.SetUniform3f("u_SpotLights[0].position", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);
		shader.SetUniform3f("u_SpotLights[0].direction", camFrontRealTime.x, camFrontRealTime.y, camFrontRealTime.z);

		renderer.DrawElements(sizeof(cubeIndices) / sizeof(int), nullptr);

		/* Draw Light */
		light.OnDraw();
		Engine::Shader& lightShader = lightMaterial.GetShader();
		lightShader.SetUniformMatrix4f("u_Model", modelMatLight);
		lightShader.SetUniformMatrix4f("u_View", viewMat);
		lightShader.SetUniformMatrix4f("u_Projection", projectionMat);

		renderer.DrawElements(sizeof(lightVertices) / sizeof(int), nullptr);

		renderer.Render();
	}

	cube.Destroy();
	light.Destroy();

	cubeMaterial.Delete();
	lightMaterial.Delete();
	
	renderer.Clear();

	return 0;
}