#pragma once
#include "Model.h"
#include "Renderer.h"
#include "RenderTarget.h"
#include "Shader.h"

namespace Engine
{
    class ShadowMapPoint
    {
    private:
        RenderTarget m_RenderTarget;
        Shader m_Shader;       
        
        
        int m_Resolution = 1024;
        bool m_Debug = false;
    public:
        ShadowMapPoint();
        ShadowMapPoint(int resolution, bool debug = false);
        
        void OnCapture(Renderer& renderer) const;
        void PostCapture(Renderer& renderer) const;
        void CaptureModel(const glm::mat4& modelMatrix, Model& model, Renderer& renderer);
        void CaptureObject(const glm::mat4& modelMatrix, Object& object, Renderer& renderer);
        
        void SetCaptureView(const glm::vec3& captureWorldPosition, float fov, float aspect, float nearPlane, float farPlane);
        void SaveDepthMap(const std::string& directory) const { m_RenderTarget.SaveDepthCubeAttachment(directory); }
        
        void UpdateCapturePosition(const glm::vec3 captureWorldPosition);
        
        uint32_t GetShadowMap() const {return m_RenderTarget.GetCubeDepthBuffer();}
    };
}