#include "Type.h"

glm::mat4 Engine::GenerateModelMatrix(const Engine::Transform& transform)
{
    glm::mat4 modelMatrix{1.0f};
    
    modelMatrix = glm::translate(modelMatrix, transform.translation);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.z), glm::vec3{0.0f, 0.0f, 1.0f});
    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.y), glm::vec3{0.0f, 1.0f, 0.0f});
    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.x), glm::vec3{1.0f, 0.0f, 0.0f});
    modelMatrix = glm::scale(modelMatrix, transform.scale);
    
    return modelMatrix;
}