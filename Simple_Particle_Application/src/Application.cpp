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

	/* Posses camera */
	camController.PossesCamera(&cam);
	
	/* Asset */
	Engine::Vertex cubeVertices[24];
	Engine::vec3 centers[1] = {
		Engine::vec3{0.0f, 0.0f, 0.0f},
	};
	float widths[1] {70.0f};
	float heights[1] {70.0f};
	float depths[1] {70.0f};
	int cubeIndices[36];
	float textureSlot = 0.0f;
	const float cubeRotateSpeed = 17.0f;
	
	Engine::CreateCube<1>(centers, widths, heights, depths, cubeVertices, cubeIndices);

	for(auto& vertex : cubeVertices)
		vertex.textureSlot = textureSlot;

	/* Light */
	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);
	Engine::Vertex lightVertices[24];
	Engine::vec3 lightCenters[1] = {
		{lightPosition.x, lightPosition.y, lightPosition.z},
	};
	float widthsL[1]{ 20.0f };
	float heightsL[1]{ 20.0f };
	float depthsL[1]{ 20.0f };
	float lightColor[3]{ 0.7f, 0.7f, 0.1f };

	Engine::CreateCube<1>(lightCenters, widthsL, heightsL, depthsL, lightVertices, cubeIndices);

	for (auto& vertex : lightVertices)
	{
		vertex.color = { lightColor[0], lightColor[1], lightColor[2], 1.0f };
	}
	
	glm::mat4 modelMatLight = glm::translate(glm::mat4(1.0f), glm::vec3(70, 0, -70));
	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -170));
	glm::mat4 viewMat = cam.GetViewMatrix();
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 640.0f/360.0f, 0.1f, 3000.0f);
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));

	glm::vec3 lightWorldPos = modelMatLight * glm::vec4(lightPosition, 1.0f);

	/* Object */
	Engine::VertexArrayBuffer VAO{};
	Engine::VertexBuffer VBO{};
	Engine::IndexBuffer IBO{};

	/* Rectangle vertex attribute buffer(VAO) */
	VAO.Bind();

	/* Rectangle vertex data buffer(VBO MUST BE BIND INSIDE VAO!) */
	VBO.Bind();
	VBO.SetBufferData(sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	/* Rectangle vertex indices buffer (IBO MUST BE BIND INSIDE VAO) */
	IBO.Bind();
	IBO.SetBufferData(sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	/* Shader */
	std::string vsFilePath = "res/shader/VSShader.vert";
	std::string fsFilePath = "res/shader/FSShader.frag";
	Engine::Shader shader{ vsFilePath, fsFilePath };
	
	/* Texture */
	Engine::Texture textureAO{std::string{"res/texture/AO_Logo_Square.png"}};
	Engine::Texture textureRG{std::string{"res/texture/Roland-Garros-logo.png"}};
	Engine::Texture textureUSO{std::string{"res/texture/US_OPEN_Logo.png"}};
	Engine::Texture textureWim{ std::string{"res/texture/wimbledon-logo.png"} };
	
	int textureSlots[4] = { 0,1,2,3 };
	textureAO.Bind(0);
	textureRG.Bind(1);
	textureUSO.Bind(2);
	textureWim.Bind(3);

	Engine::VertexAtrribArray::Enable(0);
	Engine::VertexAtrribArray::Enable(1);
	Engine::VertexAtrribArray::Enable(2);
	Engine::VertexAtrribArray::Enable(3);

	Engine::VertexAtrribArray::SetPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, pos));
	Engine::VertexAtrribArray::SetPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, texCoord));
	Engine::VertexAtrribArray::SetPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, textureSlot));
	Engine::VertexAtrribArray::SetPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, normal));
	VAO.Unbind();

	/* Light */
	Engine::VertexArrayBuffer lightVAO{};
	Engine::VertexBuffer lightVBO{};
	Engine::IndexBuffer lightIBO{};

	lightVAO.Bind();

	lightVBO.Bind();
	lightVBO.SetBufferData(sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

	lightIBO.Bind();
	lightIBO.SetBufferData(sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	std::string lightVSFilePath = "res/shader/VSShaderLight.vert";
	std::string lightFSFilePath = "res/shader/FSShaderLight.frag";
	Engine::Shader lightShader{ lightVSFilePath, lightFSFilePath };

	Engine::VertexAtrribArray::Enable(0);
	Engine::VertexAtrribArray::Enable(1);
	Engine::VertexAtrribArray::SetPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, pos));
	Engine::VertexAtrribArray::SetPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, color));
	lightVAO.Unbind();

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

		/* Draw Object */
		VAO.Bind();
		shader.Use();
		
		
		/* Set shader uniform */
		glm::vec3 camPosRealTime = cam.GetPosition();
		shader.SetUniformMatrix4f("u_Model", modelMat);
		shader.SetUniformMatrix4f("u_View", viewMat);
		shader.SetUniformMatrix4f("u_Projection", projectionMat);
		shader.SetUniformMatrix3f("u_NormalMat", normalMat);
		shader.SetUniform1iv("u_Texture", sizeof(textureSlots) / sizeof(int), textureSlots);
		shader.SetUniform3f("u_CameraPosition", camPosRealTime.x, camPosRealTime.y, camPosRealTime.z);
		shader.SetUniform3f("u_LightPosition", lightWorldPos.x, lightWorldPos.y, lightWorldPos.z);
		shader.SetUniform3f("u_LightColor", lightColor[0], lightColor[1], lightColor[2]);
		
		renderer.DrawElements(sizeof(cubeIndices) / sizeof(int), nullptr);
		shader.UnUse();

		/* Draw Light */
		lightVAO.Bind();
		lightShader.Use();

		lightShader.SetUniformMatrix4f("u_Model", modelMatLight);
		lightShader.SetUniformMatrix4f("u_View", viewMat);
		lightShader.SetUniformMatrix4f("u_Projection", projectionMat);

		renderer.DrawElements(sizeof(lightVertices) / sizeof(int), nullptr);
		lightShader.UnUse();

		renderer.Render();
	}

	textureAO.Delete();
	textureRG.Delete();
	textureWim.Delete();
	textureUSO.Delete();
	
	shader.Delete();
	lightShader.Delete();
	
	IBO.DeleteBuffer();
	VBO.DeleteBuffer();
	VAO.Delete();
	
	lightIBO.DeleteBuffer();
	lightVBO.DeleteBuffer();
	lightVAO.Delete();
	
	renderer.Clear();

	return 0;
}