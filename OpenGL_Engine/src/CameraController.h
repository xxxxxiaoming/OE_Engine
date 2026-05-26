#pragma once
#include <glfw/glfw3.h>
#include <unordered_map>
#include <string>
#include "Camera.h"
#include "Type.h"

namespace Engine
{
	class CameraController
	{
	private:
		const GLFWwindow* m_Window = nullptr;
		float m_MoveSpeed = 0.0f;
		float m_RotateSpeed = 0.0f;
		Camera* m_Camera = nullptr;

		std::unordered_map<Operations, int> m_KeyMap;

		void OperateCamera(const Operations op, float deltaTime) const;
	public:
		CameraController(const GLFWwindow* window, float moveSpeed, float rotateSpeed);
		~CameraController() = default;

		void BindKey(const Operations op, int key);
		void UnBindKey(const Operations op);

		void SetMoveSpeed(float moveSpeed) { m_MoveSpeed = moveSpeed; }
		void SetRotateSpeed(float rotateSpeed) { m_RotateSpeed = rotateSpeed; }

		void PossessCamera(Camera* camera);
		void UnPossesCamera();
		void ProcessInput(float deltaTime) const;
	};
}