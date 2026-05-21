#pragma once
#include "Shader.h"
#include "Model.h"
#include "ShadowMap.h"
#include "Type.h"

namespace  Engine
{
    class PhongLight
    {
    private:
        Shader m_Shader;
        ShadowMap m_ShadowMap;
        int m_PointLightIndex = 0;
        int m_SpotLightIndex = 0;
        
        std::unordered_map<std::string, Model*> m_Models;
        std::unordered_map<std::string, Object*> m_Objects;
        bool m_ShadowMapCaptured = false;
        
        void BlockModelInternal(Model* model) {model->BindShader(nullptr); model->DisableLight();}
        void BlockObjectInternal(Object* object) {object->m_Material.BindShader(nullptr);object->DisableLight();}
        void UnblockModelInternal(Model* model) {model->BindShader(&m_Shader);model->EnableLight();}
        void UnblockObjectInternal(Object* object) {object->m_Material.BindShader(&m_Shader);object->EnableLight();}
        void GenerateShadowMapInternal(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    public:
        PhongLight(int shadowMapResolution = 1024);
        
        void TurnOn(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void TurnOff(Renderer& renderer) const;
        
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
        
        void ConfigShadowMapCaptureView(glm::vec3 capturePosition, glm::vec3 captureLookAt, float viewLeft, float viewRight, float viewBottom, float viewTop, float viewNear, float viewFar);
        
        void AddModel(const std::string& name, Model* model);
        void AddObject(const std::string& name, Object* object);
        
        void RemoveModel(const std::string& name);
        void RemoveObject(const std::string& name);
    };
}