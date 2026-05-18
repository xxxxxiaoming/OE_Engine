#include "RenderExperments.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static void BindKeys(Engine::CameraController& camController)
{
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
}

static void ConfigPhongLight(Engine::PhongLight& phongLight, const Engine::vec3& pointLightWorldPosition, Engine::Camera& camera)
{
	/* 光照配置 */
	phongLight.EnableDirectionLight();								// 开启方向光源
	phongLight.ConfigDirectionLight(
		Engine::vec3{1.0f, 1.0f, 1.0f},
		Engine::vec3{0.5f,0.5f,0.5f},
		Engine::vec3{1.0f,1.0f,1.0f},
		Engine::vec3{0.7f,0.7f,0.7f}
		);
	
	phongLight.AddPointLight(
		pointLightWorldPosition, 
		Engine::vec3{0.1f, 0.1f, 0.1f}, 
		Engine::vec3{0.5f,0.5f,0.5f},
		Engine::vec3{1.0f, 1.0f, 1.0f},
		1.0f, 0.001f, 0.0001f
		);															// 使用1个点光源
	
	// glm::vec3 cameraWorldPosition = camera.GetPosition();
	// glm::vec3 cameraLookDirection = camera.GetFront();
	// phongLight.AddSpotLight(
	// 	Engine::vec3{cameraWorldPosition.x, cameraWorldPosition.y, cameraWorldPosition.z},
	// 	Engine::vec3{cameraLookDirection.x, cameraLookDirection.y, cameraLookDirection.z},
	// 	Engine::vec3{0.1f, 0.1f, 0.1f},
	// 	Engine::vec3{0.5f, 0.5f, 0.5f},
	// 	Engine::vec3{0.3f, 0.3f, 0.3f},
	// 	5.0f, 10.0f, 1.0f, 0.001f, 0.0001f
	// 	);															// 使用1个聚光源
}

void GenerateRocksModelMatrices(int amount, float minOrbitRadius, float maxOrbitRadius, float zOffset, glm::mat4* matrices)
{
	for (int count = 0; count < amount; count++)
	{
		float rotateAngle = Engine::Random::Float(0.0f, 360.0f);
		float radius = Engine::Random::Float(minOrbitRadius, maxOrbitRadius);
		float scaleFactor = Engine::Random::Float(0.2f, 0.3f);
		glm::mat4 modelMatrix{1.0f};
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, zOffset));
		modelMatrix = glm::rotate(modelMatrix, rotateAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(radius, 0.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		
		matrices[count] = modelMatrix;
	}
}

void AdvancedLighting(Engine::Renderer& renderer)
{
	/* Camera and controller */
	glm::vec3 camPostion = glm::vec3(0.0f, 0.0f, 0.0f);
	Engine::Camera cam{ camPostion, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 20.0f, 10.0f };

	/* Bind keys */
	BindKeys(camController);

	/* Possess camera */
	camController.PossessCamera(&cam);
	
	/* Phong Light */
	Engine::PhongLight phongLight;
	Engine::vec3 pointLightPosition{0.0f, 50.0f, 0.0f};
	ConfigPhongLight(phongLight, pointLightPosition, cam);
	
	Engine::Vertex floorVertices[4];
	Engine::vec3 positions[1] = {
		Engine::vec3{-100.0f, -100.0f, 0.0f}
	};
	
	float width[1] = {
		200.0f
	};
	float height[1] = {
		200.0f
	};
	
	uint32_t floorIndices[6];
	
	std::string assetPath{"res/"};
	Engine::createRectangle(positions, width, height, floorVertices, floorIndices);
	Engine::Object floor{floorVertices, floorIndices, 4, 6, assetPath};
	
	floor.m_TexturesDiffuse.reserve(1);
	floor.m_TextureSpecular.reserve(1);
	floor.m_TexturesDiffuse.emplace_back("res/texture/container2.png");
	floor.m_TextureSpecular.emplace_back("res/texture/container2_specular.png");
	
	int floorDiffuseSlot[1] = {0};
	int floorSpecularSlot[1] = {1};
	floor.m_Material.BindDiffuseSlots(floorDiffuseSlot, 1);
	floor.m_Material.BindSpecularSlots(floorSpecularSlot, 1);
	floor.EnableLight();
	
	phongLight.AddObject(floor);
	
	glm::mat4 projectionMatrix{glm::perspective(glm::radians(45.0f), 640.0f / 360.0f, 0.1f, 3000.0f)};
	glm::mat4 floorModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -140.0f));
	glm::mat4 normalMat = glm::transpose(glm::inverse(glm::mat3(floorModelMatrix)));
	floorModelMatrix = glm::rotate(floorModelMatrix, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	double lastFrameTime = glfwGetTime();
	while (!renderer.CheckWindowShouldClose())
	{
		double thisFrameTime = glfwGetTime();
		double deltaTime = thisFrameTime - lastFrameTime;
		lastFrameTime = thisFrameTime;
		
		camController.ProcessInput(static_cast<float>(deltaTime));
		
		glm::mat4 viewMat = cam.GetViewMatrix();
		
		glm::vec3 camPosRealTime = cam.GetPosition();

		phongLight.ConfigMVPMatrix(floorModelMatrix, viewMat, projectionMatrix);
		phongLight.ConfigNormalMatrix(normalMat);
		phongLight.ConfigCameraWorldPosition(camPosRealTime);
		
		renderer.OnRender();
		
		phongLight.TurnOn();
		floor.OnDraw();
		renderer.DrawElements(6, nullptr);
		phongLight.TurnOff();
		
		renderer.Render();
	}
}

void InstanceExperiment(Engine::Renderer& renderer)
{
	/* Camera and controller */
	glm::vec3 camPostion = glm::vec3(0.0f, 0.0f, 0.0f);
	Engine::Camera cam{ camPostion, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 20.0f, 10.0f };

	/* Bind keys */
	BindKeys(camController);

	/* Possess camera */
	camController.PossessCamera(&cam);
	
	/* Load planet model */
	Engine::Model planet{"res/assets/planet/planet.obj"};
	Engine::Model rock{"res/assets/rock/rock.obj"};
	
	planet.DisableLight();
	rock.DisableLight();
	
	Engine::Shader planetShader{std::string{"res/shader/BasicVS.vert"}, std::string{"res/shader/BasicFS.frag"}};
	Engine::Shader rockShader{std::string{"res/shader/AsteroidVS.vert"}, std::string{"res/shader/BasicFS.frag"}};
	
	planet.BindShader(&planetShader);
	rock.BindShader(&rockShader);
	
	int planetDiffuseSlot[1]  = {0};
	int planetSpecularSlot[1]  = {1};
	planet.BindDiffuseSlot(planetDiffuseSlot, 1);
	planet.BindSpecularSlot(planetSpecularSlot, 1);
	
	int rockDiffuseSlot[1]  = {2};
	int rockSpecularSlot[1]  = {3};
	rock.BindDiffuseSlot(rockDiffuseSlot, 1);
	rock.BindSpecularSlot(rockSpecularSlot, 1);
	
	float planetRotateSpeed = 7.0f;
	float planetRotateAngle = 0.0f;
	float zOffset = -10.0f;
	
	const int asteroidAmount = 7777;
	glm::mat4 rocksModelMatrices[asteroidAmount];
	GenerateRocksModelMatrices(asteroidAmount, 7.0f, 177.0f, zOffset, rocksModelMatrices);
	
	Engine::VertexBuffer rockModelMatricesVBO{};
	rockModelMatricesVBO.Bind();
	rockModelMatricesVBO.SetBufferData(sizeof(rocksModelMatrices), rocksModelMatrices, GL_STATIC_DRAW);
	
	rock.BindInstancedVertexAttrib(5, 4, GL_FLOAT, sizeof(glm::mat4), 0, 1);
	rock.BindInstancedVertexAttrib(6, 4, GL_FLOAT, sizeof(glm::mat4), sizeof(glm::vec4), 1);
	rock.BindInstancedVertexAttrib(7, 4, GL_FLOAT, sizeof(glm::mat4), 2 * sizeof(glm::vec4), 1);
	rock.BindInstancedVertexAttrib(8, 4, GL_FLOAT, sizeof(glm::mat4), 3 * sizeof(glm::vec4), 1);
	
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 640.0f / 360.0f, 0.1f, 3000.0f);
	
	double lastFrameTime = glfwGetTime();
	while (!renderer.CheckWindowShouldClose())
	{
		double thisFrameTime = glfwGetTime();
		float deltaTime = static_cast<float>(thisFrameTime - lastFrameTime);
		lastFrameTime = thisFrameTime;
		
		planetRotateAngle += planetRotateSpeed * deltaTime;
		
		planetRotateAngle = planetRotateAngle >= 360.0f ? 0.0f : planetRotateAngle;
		
		camController.ProcessInput(deltaTime);
		
		glm::mat4 viewMatrix = cam.GetViewMatrix();
		glm::mat4 planetModelMatrix = glm::mat4(1.0f);
		
		planetModelMatrix = glm::translate(planetModelMatrix, glm::vec3(0.0f, 0.0f, zOffset));
		planetModelMatrix = glm::rotate(planetModelMatrix, glm::radians(planetRotateAngle), glm::vec3(0.0, 1.0f, 0.0f));
		
		
		renderer.OnRender();
		
		planetShader.Use();
		planetShader.SetUniformMatrix4f("u_Model", planetModelMatrix);
		planetShader.SetUniformMatrix4f("u_View", viewMatrix);
		planetShader.SetUniformMatrix4f("u_Projection", projectionMatrix);
		planetShader.SetUniform1i("u_Diffuse", 0);
		planet.Draw(renderer);
		
		rockShader.Use();
		rockShader.SetUniformMatrix4f("u_View", viewMatrix);
		rockShader.SetUniformMatrix4f("u_Projection", projectionMatrix);
		rockShader.SetUniform1i("u_Diffuse", 2);
		rock.DrawInstanced(renderer, asteroidAmount);
		
		renderer.Render();
	}
}

void RenderTargetExperiment(Engine::Renderer& renderer)
{
	/* Camera and controller */
	glm::vec3 camPostion = glm::vec3(0.0f, 0.0f, 0.0f);
	Engine::Camera cam{ camPostion, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 20.0f, 10.0f };

	/* Bind keys */
	BindKeys(camController);

	/* Possess camera */
	camController.PossessCamera(&cam);
	
	/* Phong Light*/
	Engine::PhongLight phongLight;
	
	/* Sky Box */
	Engine::SkyBox skyBox("res/texture/skybox/", ".jpg");

	/* Load Mode*/
	std::string modelPath{ std::string{ "res/assets/backpack/backpack.obj" } };
	Engine::Model model{ modelPath };
	
	// Sky Box Reflect
	std::string skyBoxReflctVSFile = "res/shader/VSShader.vert";
	std::string skyBoxReflctFSFile = "res/shader/SkyBoxReflectFShader.frag";
	Engine::Shader skyBoxReflectShader{skyBoxReflctVSFile, skyBoxReflctFSFile};
	model.BindShader(&skyBoxReflectShader);
	model.DisableLight();
	
	// phongLight.AddModel(model);

	int diffuseSlots[1] = { 0 };
	int specularSlots[1] = { 1 };
	model.BindDiffuseSlot(diffuseSlots, 1);
	model.BindSpecularSlot(specularSlots, 1);

	float modelRotateSpeed = 0.0f;
	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);
	glm::mat4 modelMatLight = glm::translate(glm::mat4(1.0f), glm::vec3(7, 0, 0));
	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10));
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 640.0f / 360.0f, 0.1f, 3000.0f);
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	glm::vec3 lightWorldPos = modelMatLight * glm::vec4(lightPosition, 1.0f);
	
	ConfigPhongLight(phongLight, Engine::vec3{lightWorldPos.x, lightWorldPos.y, lightWorldPos.z}, cam);
	
	Engine::RenderTarget renderTarget{1280, 720};
	renderTarget.CreateColorAttachment();
	renderTarget.CreateRenderBuffer();
	assert(renderTarget.CheckFramebufferStatus());
	
	Engine::vec3 rectPos[1] = { Engine::vec3{-1.0f, -1.0f, 0.0f} };
	Engine::Vertex vertices[4];
	uint32_t indices[6];
	
	
	float width[1] = { 2.0f };
	float height[1] = { 2.0f };
	std::string assetDirector{ "res/texture" };
	
	Engine::createRectangle(rectPos, width, height, vertices, indices);
	Engine::Object rectObj{ vertices, indices, 4, 6, assetDirector };
	
	rectObj.DisableLight();
	
	std::string vsShaderPathRect = "res/shader/FBVSShader.vert";
	std::string fsShaderPathRect = "res/shader/FSPostProcessShader.frag";
	Engine::Shader shaderRect{ vsShaderPathRect, fsShaderPathRect };
	Engine::Material matForRect;
	matForRect.BindShader(&shaderRect);
	
	double lastFrameTime = glfwGetTime();
	while (!renderer.CheckWindowShouldClose())
	{
		double currentFrame = glfwGetTime();
		double deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;
		
		camController.ProcessInput(static_cast<float>(deltaTime));
		
		modelMat = glm::rotate(modelMat, glm::radians(static_cast<float>(deltaTime) * modelRotateSpeed), glm::vec3{ 0, 1.0f, 0 });
		glm::mat4 viewMat = cam.GetViewMatrix();
		normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));

		renderTarget.BindFramebuffer();
		
		// clear render buffer before drawing
		renderer.OnRender();

		glm::vec3 camPosRealTime = cam.GetPosition();
		glm::vec3 camFrontRealTime = cam.GetFront();

		phongLight.ConfigMVPMatrix(modelMat, viewMat, projectionMat);
		phongLight.ConfigNormalMatrix(normalMat);
		phongLight.ConfigCameraWorldPosition(camPosRealTime);
		

		/* 每帧更新聚光源的位置，模拟控制手电筒^-^ */
		// phongLight.ConfigSpotLightPosition(0, Engine::vec3{camPosRealTime.x, camPosRealTime.y, camPosRealTime.z});
		// phongLight.ConfigSpotLightDirection(0, Engine::vec3{camFrontRealTime.x, camFrontRealTime.y, camFrontRealTime.z});
		
		// Draw Sky box
		skyBox.SetVPMatrix(glm::mat4(glm::mat3(viewMat)), projectionMat);
		skyBox.Draw();
		
		// phongLight.TurnOn();
		GLCALL(glActiveTexture(GL_TEXTURE0));
		GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.GetSkyBoxCubeMap()));
		skyBoxReflectShader.Use();
		skyBoxReflectShader.SetUniform1i("u_CubeMap", 0);
		skyBoxReflectShader.SetUniformMatrix4f("u_Model", modelMat);
		skyBoxReflectShader.SetUniformMatrix4f("u_View", viewMat);
		skyBoxReflectShader.SetUniformMatrix4f("u_Projection", projectionMat);
		skyBoxReflectShader.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);
		skyBoxReflectShader.SetUniform3f("u_LightPosition", lightWorldPos.x, lightWorldPos.y, lightWorldPos.z);
		skyBoxReflectShader.SetUniformMatrix3f("u_NormalMat", normalMat);
		model.Draw(renderer);
		// phongLight.TurnOff();
		
		renderTarget.UnbindFramebuffer();
		
		// clear buffer before drawing
		renderer.OnRender();
		
		uint32_t texture = renderTarget.GetTextureBuffer();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		matForRect.UseMaterial();
		rectObj.OnDraw();
		renderer.DrawElements(static_cast<int>(rectObj.GetIndexCount()), nullptr);
		
		renderer.Render();
	}
}

void StencilTestExperiment(const std::string& path, Engine::Renderer& renderer)
{
	/* Camera and controller */
	glm::vec3 camPostion = glm::vec3(0.0f, 0.0f, 0.0f);
	Engine::Camera cam{ camPostion, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 5.0f, 10.0f };

	/* Bind keys */
	BindKeys(camController);
	
	/* Possess camera */
	camController.PossessCamera(&cam);
	
	/* Phong Light*/
	Engine::PhongLight phongLight;

	/* Load Model*/
	std::string modelPath{ path };
	Engine::Model model{ modelPath };
	
	std::string vsShaderFile = "res/shader/VSShader.vert";
	std::string fsShaderFrameFile = "res/shader/DepthVisualizationShader.frag";
	Engine::Shader shaderFrame{ vsShaderFile, fsShaderFrameFile };

	int diffuseSlots[1] = { 0 };
	int specularSlots[1] = { 1 };
	model.BindDiffuseSlot(diffuseSlots, 1);
	model.BindSpecularSlot(specularSlots, 1);

	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);
	glm::mat4 modelMatLight = glm::translate(glm::mat4(1.0f), glm::vec3(7, 0, 0));
	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10));
	glm::mat4 viewMat = cam.GetViewMatrix();
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 640.0f / 360.0f, 0.1f, 300.0f);
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	glm::vec3 lightWorldPos = modelMatLight * glm::vec4(lightPosition, 1.0f);

	glm::mat4 modelMatFrame = glm::scale(modelMat, glm::vec3(1.2f, 1.2f, 1.2f));
	
	/* 光照配置 */
	phongLight.EnableDirectionLight();								// 开启方向光源
	phongLight.ConfigDirectionLight(
		Engine::vec3{1.0f, 1.0f, 1.0f},
		Engine::vec3{0.5f,0.5f,0.5f},
		Engine::vec3{1.0f,1.0f,1.0f},
		Engine::vec3{0.7f,0.7f,0.7f}
		);
	

	renderer.EnebleStencilTest();

	double lastFrame = glfwGetTime();
	while (!renderer.CheckWindowShouldClose())
	{
		double deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		/* Keyboard input */
		camController.ProcessInput(static_cast<float>(deltaTime));

		/* Transform matrix */
		modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -10.0));
		modelMat = glm::rotate(modelMat, glm::radians(static_cast<float>(deltaTime * 0.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatFrame = glm::scale(modelMat, glm::vec3(1.07f, 1.07f, 1.07f));
		
		glm::mat4 viewMat = cam.GetViewMatrix();
		normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));

		renderer.OnRender();
		
		glm::vec3 camPosRealTime = cam.GetPosition();
		glm::vec3 camFrontRealTime = cam.GetFront();
		
		phongLight.ConfigMVPMatrix(modelMat, viewMat, projectionMat);
		phongLight.ConfigNormalMatrix(normalMat);
		phongLight.ConfigCameraWorldPosition(camPosRealTime);

		/* 每帧更新聚光源的位置，模拟控制手电筒^-^ */
		phongLight.ConfigSpotLightPosition(0, Engine::vec3{camPosRealTime.x, camPosRealTime.y, camPosRealTime.z});
		phongLight.ConfigSpotLightDirection(0, Engine::vec3{camFrontRealTime.x, camFrontRealTime.y, camFrontRealTime.z});

		renderer.SetStencilFunc(GL_ALWAYS, 1, 0xFF);
		renderer.SetStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		
		phongLight.AddModel(model);
		phongLight.TurnOn();
		model.Draw(renderer);
		phongLight.TurnOff();

		model.BindShader(&shaderFrame);
		
		shaderFrame.Use();

		shaderFrame.SetUniformMatrix4f("u_Model", modelMatFrame);
		shaderFrame.SetUniformMatrix4f("u_View", viewMat);
		shaderFrame.SetUniformMatrix4f("u_Projection", projectionMat);
		shaderFrame.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);

		renderer.SetStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		renderer.SetStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		model.Draw(renderer);

		renderer.Render();
	}
}

void DrawASimpleHouseUsingGS(Engine::Renderer& renderer)
{
	/* Draw a simple house by using geometry shader */
	Engine::Vertex vertices[] = {
		Engine::Vertex{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.7f, 1.0f}, 0.0f},
		Engine::Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.7f, 1.0f}, 0.0f},
		Engine::Vertex{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.7f, 1.0f}, 0.0f},
		Engine::Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.7f, 1.0f}, 0.0f},
	};
	uint32_t indices[] = {
		0, 1, 2, 3
	};
	
	std::string assetDirector = "res/";
	Engine::Object houseObject{vertices, indices, 4, 4, assetDirector};
	
	houseObject.DisableLight();
	
	std::string vsFile = "res/shader/SimpleHouseVS.vert";
	std::string gsFile = "res/shader/SimpleHouseGS.geom";
	std::string fsFile = "res/shader/SimpleHouseFS.frag";
	Engine::Shader houseShader{vsFile, fsFile, gsFile};
	Engine::Material houseMaterial;
	
	houseMaterial.BindShader(&houseShader);
	
	while (!renderer.CheckWindowShouldClose())
	{
		houseMaterial.UseMaterial();
		
		renderer.OnRender();
		houseObject.OnDraw();
		renderer.DrawElements(4, nullptr, GL_POINTS);
		renderer.Render();
	}
	
	houseObject.Destroy();
	houseShader.Delete();
}
