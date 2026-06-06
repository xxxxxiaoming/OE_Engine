#pragma once
#include "Shader.h"
#include "Model.h"
#include "ShadowMapDirection.h"
#include "ShadowMapPoint.h"
#include "Type.h"

namespace  Engine
{
    /* 
     * 4个点光源的深度cubemap依次绑定到1，2，3，4这四个纹理槽位
     * 0，1，2，3，4这个5个纹理槽位是Engine保留的，不会被 Application 占用
     */
    class PhongLight
    {
    private:
        Shader m_ShaderLight;
        Shader m_ShaderLightForward;
        Shader m_ShaderGBuffer;
        Shader m_ShaderSSAO;
        Shader m_ShaderSSAOSmoth;
        ShadowMapDirection m_ShadowMap;
        std::vector<ShadowMapPoint>  m_ShadowMapPoint;
        int m_PointLightIndex = 0;
        int m_SpotLightIndex = 0;
        
        bool m_ShadowMapCaptured = false;
        bool m_UseDeffered = false;
        
        int m_ShadowMapResolution = 1024;
        std::vector<bool> m_PointLightsNeedCapture;
        
        std::unordered_map<std::string, Model*> m_Models;
        std::unordered_map<std::string, Object*> m_Objects;
        
        std::unordered_map<std::string, Object*> m_OpaqueObjects;
        std::unordered_map<std::string, Object*> m_MaskedObjects;
        std::unordered_map<std::string, Object*> m_TransparentObjects;
        
        RenderTarget m_RTGBuffer;
        RenderTarget m_RTLight;
        RenderTarget m_RTSSAO;
        RenderTarget m_RTSSAOSmoth;
        Object m_RenderRect;
        
        uint32_t m_SSAONoiseTexture = 0;
        
        void BlockModelInternal(Model* model) {model->BindShader(nullptr); model->DisableLight();}
        void BlockObjectInternal(Object* object) {object->m_Material.BindShader(nullptr);object->DisableLight();}
        void UnblockModelInternal(Model* model);
        void UnblockObjectInternal(Object* object);
        void GenerateShadowMapInternal(Renderer& renderer);
        void GenerateShadowMapPointInternal(Renderer& renderer);
        void GenerateSpecificShadowMapPointInternal(int index, Renderer& renderer);
        void ForwardRenderInternal(const Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void DeferredRenderInternal(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    public:
        PhongLight(int shadowMapResolution = 1024, bool bDeffered = false);
        
        virtual void TurnOn(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        virtual void TurnOff(Renderer& renderer) const;
        
        
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
        void ConfigShadowMapPointCaptureView(int index, const glm::vec3& captureWorldPosition, float fov, float aspect, float nearPlane, float farPlane);
        
        void AddModel(const std::string& name, Model* model);
        void AddObject(const std::string& name, Object* object);
        
        void RemoveModel(const std::string& name);
        void RemoveObject(const std::string& name);
        
        RenderTarget& GetFinalRenderTarget() { return m_RTLight;}
    };
}