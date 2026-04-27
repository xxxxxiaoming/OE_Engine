#include "CameraController.h"

Engine::CameraController::CameraController(const GLFWwindow* window, float moveSpeed, float rotateSpeed) :
	m_Window(window), m_MoveSpeed(moveSpeed), m_RotateSpeed(rotateSpeed)
{
	/* Unbind All Keys when initializing */
	m_KeyMap[Operations::MoveUp] = -1;
	m_KeyMap[Operations::MoveDown] = -1;
	m_KeyMap[Operations::MoveLeft] = -1;
	m_KeyMap[Operations::MoveRight] = -1;
	m_KeyMap[Operations::MoveForward] = -1;
	m_KeyMap[Operations::MoveBackward] = -1;
	m_KeyMap[Operations::LookUp] = -1;
	m_KeyMap[Operations::LookDown] = -1;
	m_KeyMap[Operations::LookLeft] = -1;
	m_KeyMap[Operations::LookRight] = -1;
}

void Engine::CameraController::BindKey(const Operations op, int key)
{
	m_KeyMap[op] = key;
}

void Engine::CameraController::UnBindKey(const Operations op)
{
	m_KeyMap[op] = -1;
}

void Engine::CameraController::PossessCamera(Camera* camera)
{
	if (camera->m_Possessed)
		return;
	
	if (m_Camera != nullptr)
	{
		std::lock_guard<std::mutex> lock(m_Camera->m_Mutex);
		m_Camera->m_Possessed = false;
	}
	
	std::lock_guard<std::mutex> lock(camera->m_Mutex);
	m_Camera = camera;
	m_Camera->m_Possessed = true;
}

void Engine::CameraController::UnPossesCamera()
{
	if (m_Camera == nullptr)
		return;

	std::lock_guard<std::mutex> lock(m_Camera->m_Mutex);
	m_Camera->m_Possessed = false;
	m_Camera = nullptr;
}

void Engine::CameraController::ProcessInput(float deltaTime) const
{
	if(m_Camera == nullptr)
		return;
	for (auto pair : m_KeyMap)
	{
		if (pair.second != -1)
		{
			if (glfwGetKey(const_cast<GLFWwindow*>(m_Window), pair.second) == GLFW_PRESS)
			{
				OperateCamera(pair.first, deltaTime);
			}
		}
	}
}

void Engine::CameraController::OperateCamera(const Operations op, float deltaTime) const
{
	std::lock_guard<std::mutex> lock(m_Camera->m_Mutex);
	switch (op)
	{
	case Operations::MoveUp:
		m_Camera->MoveUp(m_MoveSpeed * deltaTime);
		break;
	case Operations::MoveDown:
		m_Camera->MoveUp(-m_MoveSpeed * deltaTime);
		break;
	case Operations::MoveForward:
		m_Camera->MoveForward(m_MoveSpeed * deltaTime);
		break;
	case Operations::MoveBackward:
		m_Camera->MoveForward(-m_MoveSpeed * deltaTime);
		break;
	case Operations::MoveRight:
		m_Camera->MoveRight(m_MoveSpeed * deltaTime);
		break;
	case Operations::MoveLeft:
		m_Camera->MoveRight(-m_MoveSpeed * deltaTime);
		break;
	case Operations::LookUp:
		m_Camera->LookUp(m_RotateSpeed * deltaTime);
		break;
	case Operations::LookDown:
		m_Camera->LookUp(-m_RotateSpeed * deltaTime);
		break;
	case Operations::LookLeft:
		m_Camera->LookLeft(m_RotateSpeed * deltaTime);
		break;
	case Operations::LookRight:
		m_Camera->LookLeft(-m_RotateSpeed * deltaTime);
		break;
	default:
		break;
	}
}
