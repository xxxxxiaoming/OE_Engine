#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mutex>

namespace Engine
{
    class Camera
    {
    private:
        glm::vec3 m_Position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 m_Up = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        glm::vec3 m_Front = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
        glm::vec3 m_Right = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        float m_MoveSpeed = 0.0f;
        float m_RotateSpeed = 0.0f;
    public:
		bool m_Possessed = false;
        std::mutex m_Mutex;
        
        Camera(glm::vec3 position, glm::mat4 rotate);
        Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
        Camera(glm::vec3 position, float pitch, float yaw, float roll);
    
        glm::mat4 GetViewMatrix() const;
        
        inline void SetSpeed(float moveSpeed, float rotateSpeed) {  m_MoveSpeed = moveSpeed; m_RotateSpeed = rotateSpeed; };
        
        /* Movements */
        void MoveForward(float distance);
        void MoveRight(float distance);
        void MoveUp(float distance);
        
        void LookUp(float angle);
        void LookLeft(float angle);
    };
}