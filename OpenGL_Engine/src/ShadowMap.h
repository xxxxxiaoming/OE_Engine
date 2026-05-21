#pragma once
#include "Camera.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "Model.h"


namespace Engine
{
    class ShadowMap
    {
    private:
        Engine::RenderTarget m_RenderTarget;
        Engine::Shader m_Shader;
        
        int m_Resolution = 1024;
        bool m_Debug = false;
    
    public:
        ShadowMap(int resolution, bool debug = false);
        // ~ShadowMap();
        
        
        
        void OnCapture(const Renderer& renderer) const;
        void PostCapture(Renderer& renderer) const;
        void CaptureModel(const glm::mat4& modelMatrix, Model& model, Renderer& renderer);
        void CaptureObject(const glm::mat4& modelMatrix, Object& object, const Renderer& renderer);
        
        void SetCaptureView(const Camera& captureCamera, const glm::mat4& projectionMatrix);
        
        uint32_t GetShadowMap() const {return m_RenderTarget.GetDepthBuffer();}
        
        void SaveDepthMap(const std::string& path) const;
    };
}
