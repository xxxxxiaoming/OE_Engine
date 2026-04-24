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
	Engine::Renderer renderer{ 1280, 640, "Hello OpenGL!" };
	
	renderer.EnableBlend();
	renderer.EnableDepthTest();

	/* Camera and controller */
	Engine::Camera cam{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)};
	Engine::CameraController camController{ renderer.GetGLFWwinow(), 7.0f, 1.7f};

	/* Bind keys */
	camController.BindKey(Engine::Operations::MoveUp, GLFW_KEY_W);
	camController.BindKey(Engine::Operations::MoveDown, GLFW_KEY_S);
	camController.BindKey(Engine::Operations::MoveLeft, GLFW_KEY_A);
	camController.BindKey(Engine::Operations::MoveRight, GLFW_KEY_D);
	camController.BindKey(Engine::Operations::MoveForward, GLFW_KEY_Q);
	camController.BindKey(Engine::Operations::MoveBackward, GLFW_KEY_E);
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
	float textureSlot = 3.0f;
	const float cubeRotateSpeed = 0.0f;
	
	Engine::CreateCube<1>(centers, widths, heights, depths, cubeVertices, cubeIndices);

	for(auto& vertex : cubeVertices)
		vertex.textureSlot = textureSlot;
	
	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -200));
	glm::mat4 viewMat = cam.GetViewMatrix();
	glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 640.0f/360.0f, 0.1f, 300.0f);
	glm::mat4 transformMat = projectionMat * viewMat * modelMat;

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
	shader.Use();
	
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

	Engine::VertexAtrribArray::SetPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, pos));
	Engine::VertexAtrribArray::SetPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, texCoord));
	Engine::VertexAtrribArray::SetPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Engine::Vertex), offsetof(Engine::Vertex, textureSlot));
	VAO.Unbind();
	
	VAO.Bind();
	double lastFrame = glfwGetTime();
	
	while (!renderer.CheckWindowShouldClose())
	{
		/* Render here */
		double deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();
		
		/* Keyboard input */
		camController.ProcessInput(static_cast<float>(deltaTime));
		
		/* Transform matrix */
		modelMat = glm::rotate(modelMat, glm::radians(static_cast<float>(deltaTime) * cubeRotateSpeed), glm::vec3{0, 1.0f, 0});
		transformMat = projectionMat * (cam.GetViewMatrix()) * modelMat;
		
		/* Set shader uniform */
		shader.SetUniformMatrix4f("u_MVP", transformMat);
		shader.SetUniform1iv("u_Texture", sizeof(textureSlots) / sizeof(int), textureSlots);
		
		/* Draw */
		renderer.Render(sizeof(cubeIndices) / sizeof(int), nullptr);
	}

	textureAO.Delete();
	textureRG.Delete();
	textureWim.Delete();
	textureUSO.Delete();
	shader.Delete();
	IBO.DeleteBuffer();
	VBO.DeleteBuffer();
	VAO.Delete();
	renderer.Clear();

	return 0;
}