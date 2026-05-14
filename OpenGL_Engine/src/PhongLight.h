#pragma once
#include "Shader.h"
#include "Model.h"
#include "Type.h"

namespace  Engine
{
    class PhongLight
    {
    private:
        Shader m_Shader;
        int m_PointLightIndex = 0;
        int m_SpotLightIndex = 0;
    public:
        PhongLight();
        
        void TurnOn() const;
        void TurnOff() const;
        
        void ConfigDirectionLight(const vec3& direction, const vec3& ambient, const vec3& diffuse, const vec3& specular);
        void EnableDirectionLight();
        void DisableDirectionLight();
        
        bool AddPointLight(const vec3& position, const vec3& ambient, const vec3& diffuse, const vec3& specular, float constant = 1.0f, float linear = 0.0f, float quadratic = 0.0f);
        bool ConfigPointLightPosition(int index, const vec3& position);
        bool ConfigPointLightColor(int index, const vec3& ambient, const vec3& diffuse, const vec3& specular);
        bool ConfigPointLightAttenuation(int index, float constant, float linear, float quadratic);
        
        bool AddSpotLight(const vec3& position, const vec3& direction, const vec3& ambient, const vec3& diffuse, const vec3& specular, float innerAngle, float outterAngle, float constant = 1.0f, float linear = 0.0f, float quadratic = 0.0f);
        bool ConfigSpotLightPosition(int index, const vec3& position);
        bool ConfigSpotLightDirection(int index, const vec3& direction);
        bool ConfigSpotLightAttenuation(int index, float constant, float linear, float quadratic);
        bool ConfigSpotLightColor(int index, const vec3& ambient, const vec3& diffuse, const vec3& specular);
        bool ConfitSpotLightScale(int index, float innerAngle, float outterAngle);
        
        void ConfigMVPMatrix(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void ConfigNormalMatrix(const glm::mat3& normalMatrix);
        void ConfigCameraWorldPosition(const glm::vec3& position);
        
        void AddModel(Model& model) { model.BindShader(&m_Shader);}
        void AddObject(Object& object) { object.m_Material.shader = &m_Shader;}
    };
}