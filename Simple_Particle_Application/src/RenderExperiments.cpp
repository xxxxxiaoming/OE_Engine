#include "RenderExperments.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define USE_DEFERRED_RENDERING

float exposure = 1.0f;
static void ProcessInput(Engine::Renderer& renderer, float deltaTime)
{
	if (glfwGetKey(const_cast<GLFWwindow*>(renderer.GetGLFWwinow()), GLFW_KEY_1) == GLFW_PRESS)
	{
		exposure = exposure + 0.1f;
	}
	else if (glfwGetKey(const_cast<GLFWwindow*>(renderer.GetGLFWwinow()), GLFW_KEY_2) == GLFW_PRESS)
	{
		exposure = exposure - 0.1f > 0.0f ? exposure - 0.1f : exposure;
	}
}

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
		Engine::vec3{-1.0f, -1.0f, -1.0f},
		Engine::vec3{0.02f,0.02f,0.02f},
		Engine::vec3{4.7f,4.7f,4.7f},
		Engine::vec3{0.1f,0.1f,0.11f}
		);
	
	// phongLight.AddPointLight(
	// 	pointLightWorldPosition, 
	// 	Engine::vec3{0.1f, 0.1f, 0.1f}, 
	// 	Engine::vec3{1.7f,1.7f,1.7f},
	// 	Engine::vec3{0.1f, 0.1f, 0.1f},
	// 	1.0f, 0.001f, 0.0001f
	// 	);															// 使用2个点光源
	
	// phongLight.AddPointLight(
	// 	Engine::vec3{pointLightWorldPosition.x + 10.0f, pointLightWorldPosition.y, pointLightWorldPosition.z},
	// 	Engine::vec3{0.1f, 0.1f, 0.1f}, 
	// 	Engine::vec3{7.17f,5.0f,5.0f},
	// 	Engine::vec3{5.5f, 5.5f, 5.5f},
	// 	1.0f, 0.001f, 0.0001f
	// 	);
	
	// glm::vec3 cameraWorldPosition = camera.GetPosition();
	// glm::vec3 cameraLookDirection = camera.GetFront();
	// phongLight.AddSpotLight(
	// 	Engine::vec3{cameraWorldPosition.x, cameraWorldPosition.y, cameraWorldPosition.z},
	// 	Engine::vec3{cameraLookDirection.x, cameraLookDirection.y, cameraLookDirection.z},
	// 	Engine::vec3{0.01f, 0.01f, 0.01f},
	// 	Engine::vec3{0.7f, 0.7f, 0.7f},
	// 	Engine::vec3{0.5f, 0.5f, 0.5f},
	// 	50.0f, 70.0f, 1.0f, 0.001f, 0.0001f
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
	glm::vec3 camPostion = glm::vec3(0.0f, 100.0f, 170.0f);
	Engine::Camera cam{ camPostion, glm::vec3(0.0f, 55.0f, -100.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 100.0f, 40.0f };
	
	/* HDR render target */
	bool bHDR = true;
	Engine::RenderTarget renderTargetHDR{1280, 720, true};
	renderTargetHDR.CreateColorAttachment();
	renderTargetHDR.CreateRenderBuffer();
	
	Engine::vec3 rectPos[1] = { Engine::vec3{-1.0f, -1.0f, 0.0f} };
	Engine::Vertex vertices[4];
	uint32_t indices[6];
	float widthHDRRect[1] = { 2.0f };
	float heightHDRRect[1] = { 2.0f };
	std::string assetDirector{ "res/texture" };
	
	Engine::createRectangle(rectPos, widthHDRRect, heightHDRRect, vertices, indices);
	Engine::Object rectObj{ vertices, indices, 4, 6, assetDirector };
	
	rectObj.DisableLight();
	
	std::string vsShaderPathRect = "res/shader/FBVSShader.vert";
	std::string fsShaderPathRect = "res/shader/FSHDR.frag";
	Engine::Shader shaderRect{ vsShaderPathRect, fsShaderPathRect };
	Engine::Material matForRect;
	matForRect.BindShader(&shaderRect);
	shaderRect.SetUniform1f("u_Exposure", exposure);
	shaderRect.SetUniform1i("u_HDR", bHDR);
	
	/* Bind keys */
	BindKeys(camController);

	/* Possess camera */
	camController.PossessCamera(&cam);
	
	/* Phong Light */
	Engine::PhongLight phongLight{1024, true};
	Engine::vec3 pointLightPosition{-70.0f, 70.0f, -50.0f};
	ConfigPhongLight(phongLight, pointLightPosition, cam);
	
	/* Shadow Map */
	glm::vec3 shadowMapCamLookAt = glm::vec3(0.0f, 0.0f, -70.0f); // 精准瞄准 Nanosuit
	glm::vec3 shadowMapCaptureCamPos = shadowMapCamLookAt + glm::vec3(150.0f, 150.0f, 150.0f); // 把光源相机拉近
	
	// phongLight.DisableDirectionLight();

	// 将投影矩阵的宽高大幅度缩小到 300x300，让 1024 贴图的像素密度暴增！
	phongLight.ConfigShadowMapCaptureView(
		shadowMapCaptureCamPos, 
		shadowMapCamLookAt, 
		-150.0f, 150.0f, // Left, Right (宽度缩减到300)
		-150.0f, 150.0f, // Bottom, Top (高度缩减到300)
		0.1f, 500.0f    // Near, Far
	);
	// phongLight.ConfigShadowMapPointCaptureView(
	// 	0,
	// 	glm::vec3{pointLightPosition.x, pointLightPosition.y, pointLightPosition.z},
	// 	90,
	// 	1.0,
	// 	0.1f, 2000.0f
	// );
	
	constexpr float sizeX = 2000.0f;
	constexpr float sizeY = 2000.0f;
	Engine::Vertex floorVertices[4];
	Engine::vec3 positions[1] = {
		Engine::vec3{-sizeX / 2.0f, -sizeY / 2.0f, 0.0f}
	};
	
	float width[1] = {
		sizeX
	};
	float height[1] = {
		sizeY
	};
	
	uint32_t floorIndices[6];
	
	std::string assetPath{"res/"};
	Engine::createRectangle(positions, width, height, floorVertices, floorIndices);
	
	for (int index = 0 ; index < 4 ; index++)
	{
		Engine::Vertex& vertex = floorVertices[index];
		vertex.texCoord.x *= 10;
		vertex.texCoord.y *= 10;
	}
	
	Engine::Transform floorTransform = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -140.0f),
		glm::vec3(-90.0f, 0.0f, 0.0f)
	};
	Engine::Object floor{floorVertices, floorIndices, 4, 6, assetPath, floorTransform};
	
	floor.m_TexturesAmbient.reserve(1);
	floor.m_TexturesDiffuse.reserve(1);
	floor.m_TextureSpecular.reserve(1);
	floor.m_TexturesAmbient.emplace_back("res/texture/brickwall.jpg");
	floor.m_TexturesDiffuse.emplace_back("res/texture/brickwall.jpg");
	floor.m_TextureSpecular.emplace_back(0xFF000000);
	floor.m_TextureNormal.emplace_back("res/texture/brickwall_normal.jpg");
	
	int floorDiffuseSlot[1] = {1};
	int floorSpecularSlot[1] = {2};
	int floorNormalSlot[1] = {3};
	floor.m_Material.BindAmbientSlots(floorDiffuseSlot, 1);
	floor.m_Material.BindDiffuseSlots(floorDiffuseSlot, 1);
	floor.m_Material.BindSpecularSlots(floorSpecularSlot, 1);
	floor.m_Material.BindNormalSlots(floorNormalSlot, 1);
	floor.EnableLight();
	floor.SetBlendMode(Engine::BlendMode::Opaque);
	
	phongLight.AddObject("floor", &floor);
	
	Engine::Transform nanosuitTransform = {
		glm::vec3(10.0f, 10.0f, 10.0f),
		glm::vec3(0.0f, 20.0f, -70.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
	};
	Engine::Model nanoSuit{"res/assets/backpack/backpack.obj", true
		, nanosuitTransform};
	
	int nanosuitAmbientSlot[1] = {4};
	int nanoSuitDiffuseSlot[1] = {5};
	int nanoSuitSpecularSlot[1] = {6};
	int nanoSuitNormalSlot[1] = {7};
	nanoSuit.BindAmbientSlot(nanosuitAmbientSlot, 1);
	nanoSuit.BindDiffuseSlot(nanoSuitDiffuseSlot, 1);
	nanoSuit.BindSpecularSlot(nanoSuitSpecularSlot, 1);
	nanoSuit.BindNormalSlot(nanoSuitNormalSlot, 1);
	
	phongLight.AddModel("nanosuit", &nanoSuit);
	
	glm::mat4 projectionMatrix{glm::perspective(glm::radians(45.0f), 640.0f / 360.0f, 0.1f, 3000.0f)};
	
	double lastFrameTime = glfwGetTime();
	while (!renderer.CheckWindowShouldClose())
	{
		double thisFrameTime = glfwGetTime();
		double deltaTime = thisFrameTime - lastFrameTime;
		lastFrameTime = thisFrameTime;
		
		camController.ProcessInput(static_cast<float>(deltaTime));
		ProcessInput(renderer, deltaTime);
		
		glm::mat4 viewMat = cam.GetViewMatrix();
		glm::vec3 camPosRealTime = cam.GetPosition();
		glm::vec3 camFrontRealTime = cam.GetFront();
		
		phongLight.ConfigCameraWorldPosition(camPosRealTime);
		// phongLight.ConfigPointLightPosition(0, Engine::vec3{camPosRealTime.x, camPosRealTime.y, camPosRealTime.z});
		
		/* 每帧更新聚光源的位置，模拟控制手电筒^-^ */
		phongLight.ConfigSpotLightPosition(0, Engine::vec3{camPosRealTime.x, camPosRealTime.y, camPosRealTime.z});
		phongLight.ConfigSpotLightDirection(0, Engine::vec3{camFrontRealTime.x, camFrontRealTime.y, camFrontRealTime.z});
		
		if (bHDR)
		{
			renderTargetHDR.BindFramebuffer();
		
			renderer.OnRender();
		
			/* Draw Scene */
			phongLight.TurnOn(renderer, viewMat, projectionMatrix);
			phongLight.TurnOff(renderer);
		
			renderTargetHDR.UnbindFramebuffer();
		
			renderer.OnRender();
		
			matForRect.UseMaterial();
			// uint32_t texture = renderTargetHDR.GetTextureBuffer();
			uint32_t texture = phongLight.GetFinalRenderTarget().GetTextureBuffer();
			glBindTextureUnit(10, texture);
			
			shaderRect.SetUniform1i("u_Diffuse", 10);
			shaderRect.SetUniform1f("u_Exposure", exposure);
			rectObj.OnDraw();
			renderer.DrawElements(rectObj.GetIndexCount(), nullptr);
		
			renderer.Render();
		}
		else
		{
			renderer.OnRender();
		
			/* Draw Scene */
			phongLight.TurnOn(renderer, viewMat, projectionMatrix);
			phongLight.TurnOff(renderer);
		
			renderer.Render();
		}
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
		
		planetShader.SetUniformMatrix4f("u_Model", planetModelMatrix);
		planetShader.SetUniformMatrix4f("u_View", viewMatrix);
		planetShader.SetUniformMatrix4f("u_Projection", projectionMatrix);
		planetShader.SetUniform1i("u_Diffuse", 0);
		planetShader.Use();
		planet.Draw(renderer);
		
		rockShader.SetUniformMatrix4f("u_View", viewMatrix);
		rockShader.SetUniformMatrix4f("u_Projection", projectionMatrix);
		rockShader.SetUniform1i("u_Diffuse", 2);
		rockShader.Use();
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

		skyBoxReflectShader.SetUniform1i("u_CubeMap", 0);
		skyBoxReflectShader.SetUniformMatrix4f("u_Model", modelMat);
		skyBoxReflectShader.SetUniformMatrix4f("u_View", viewMat);
		skyBoxReflectShader.SetUniformMatrix4f("u_Projection", projectionMat);
		skyBoxReflectShader.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);
		skyBoxReflectShader.SetUniform3f("u_LightPosition", lightWorldPos.x, lightWorldPos.y, lightWorldPos.z);
		skyBoxReflectShader.SetUniformMatrix3f("u_NormalMat", normalMat);
		skyBoxReflectShader.Use();
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
		
		phongLight.AddModel("backpack", &model);
		phongLight.TurnOn(renderer, viewMat, projectionMat);
		// model.Draw(renderer);
		phongLight.TurnOff(renderer);

		model.BindShader(&shaderFrame);

		shaderFrame.SetUniformMatrix4f("u_Model", modelMatFrame);
		shaderFrame.SetUniformMatrix4f("u_View", viewMat);
		shaderFrame.SetUniformMatrix4f("u_Projection", projectionMat);
		shaderFrame.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);

		renderer.SetStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		renderer.SetStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		
		shaderFrame.Use();
		model.Draw(renderer);
		shaderFrame.UnUse();

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
