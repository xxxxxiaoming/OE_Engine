#include "Camera.h"

Engine::Camera::Camera(const glm::vec3& position, const glm::mat4& rotate) : 
    m_Position(position)
{
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
    m_Up = translate * rotate * glm::vec4(m_Up, 1.0f);
    m_Front = translate * rotate * glm::vec4(m_Front, 1.0f);
    m_Right = translate * rotate * glm::vec4(m_Right, 1.0f);
}

Engine::Camera::Camera(const glm::vec3& position, const glm::vec3& look, const glm::vec3& up) :
    m_Position(position), m_Up(up)
{
    m_Front = glm::normalize(look - position);
    m_Right = glm::normalize(glm::cross(m_Front, m_Up));
}

Engine::Camera::Camera(const glm::vec3& position, float pitch, float yaw, float roll) :
    m_Position(position)
{
    glm::mat4 rotate = glm::mat4(1.0f);
    rotate = glm::rotate(rotate, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    rotate = glm::rotate(rotate, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    rotate = glm::rotate(rotate, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
    
    m_Up = rotate * glm::vec4(m_Up, 1.0f);
    m_Front = rotate * glm::vec4(m_Front, 1.0f);
    m_Right = rotate * glm::vec4(m_Right, 1.0f);
}

glm::mat4 Engine::Camera::GetViewMatrix() const
{
    glm::vec3 look = m_Position + m_Front;
    return glm::lookAt(m_Position, look, m_Up);
}

void Engine::Camera::MoveForward(float distance)
{
    m_Position = m_Position + m_Front * distance;
}

void Engine::Camera::MoveRight(float distance)
{
	m_Position = m_Position + m_Right * distance;
}

void Engine::Camera::MoveUp(float distance)
{
    m_Position = m_Position + m_Up * distance;
}

void Engine::Camera::LookUp(float angle)
{
    angle = glm::radians(angle);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle, m_Right);
    m_Up = rotate * glm::vec4(m_Up, 1.0f);
    m_Front = rotate * glm::vec4(m_Front, 1.0f);
}

void Engine::Camera::LookLeft(float angle)
{
    angle = glm::radians(angle);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle, m_Up);
    m_Right = rotate * glm::vec4(m_Right, 1.0f);
    m_Front = rotate * glm::vec4(m_Front, 1.0f);
}
