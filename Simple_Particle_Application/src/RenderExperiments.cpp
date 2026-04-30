#include "RenderExperments.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void LoadModelExperiment(const std::string& path, Engine::Renderer& renderer)
{
	/* Camera and controller */
	glm::vec3 camPostion = glm::vec3(0.0f, 0.0f, 0.0f);
	Engine::Camera cam{ camPostion, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 20.0f, 10.0f };

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

	/* Load Mode*/
	std::string modelPath{ path };
	Engine::Model model{ modelPath };

	std::string vsShaderFile = "res/shader/VSShader.vert";
	std::string fsShaderFile = "res/shader/PhongLightShader.frag";
	Engine::Shader shader{ vsShaderFile, fsShaderFile };
	model.BindShader(&shader);

	int diffuseSlots[1] = { 0 };
	int specularSlots[1] = { 1 };
	model.BindDiffuseSlot(diffuseSlots, 1);
	model.BindSpecularSlot(specularSlots, 1);

	float modelRotateSpeed = 0.0f;
	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);
	glm::mat4 modelMatLight = glm::translate(glm::mat4(1.0f), glm::vec3(7, 0, 0));
	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10));
	glm::mat4 viewMat = cam.GetViewMatrix();
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 640.0f / 360.0f, 0.1f, 3000.0f);
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	glm::vec3 lightWorldPos = modelMatLight * glm::vec4(lightPosition, 1.0f);

	shader.Use();

	/* 光照配置 */
	shader.SetUniform1i("u_LightConfig.enableDirectionLight", 1);	// 开启方向光源
	shader.SetUniform1i("u_LightConfig.pointLightNum", 1);			// 使用1个点光源
	shader.SetUniform1i("u_LightConfig.spotLightNum", 0);			// 使用一个聚光源

	/* 方向光配置 */
	shader.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
	shader.SetUniform3f("u_DirectionLight.color.ambient", 0.5f, 0.5f, 0.5f);
	shader.SetUniform3f("u_DirectionLight.color.diffuse", 1.0f, 1.0f, 1.0f);
	shader.SetUniform3f("u_DirectionLight.color.specular", 0.7f, 0.7f, 0.7f);

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
		modelMat = glm::rotate(modelMat, glm::radians(static_cast<float>(deltaTime) * modelRotateSpeed), glm::vec3{ 0, 1.0f, 0 });
		glm::mat4 viewMat = cam.GetViewMatrix();
		normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));

		renderer.OnRender();

		glm::vec3 camPosRealTime = cam.GetPosition();
		glm::vec3 camFrontRealTime = cam.GetFront();

		shader.Use();

		shader.SetUniformMatrix4f("u_Model", modelMat);
		shader.SetUniformMatrix4f("u_View", viewMat);
		shader.SetUniformMatrix4f("u_Projection", projectionMat);
		shader.SetUniformMatrix3f("u_NormalMat", normalMat);
		shader.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);

		/* 每帧更新聚光源的位置，模拟控制手电筒^-^ */
		shader.SetUniform3f("u_SpotLights[0].position", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);
		shader.SetUniform3f("u_SpotLights[0].direction", camFrontRealTime.x, camFrontRealTime.y, camFrontRealTime.z);

		model.Draw(renderer);
		renderer.Render();
	}
}