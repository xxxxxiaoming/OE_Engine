#pragma once
#include <cstdint>
#include <string>
#include "Shader.h"
#include "Type.h"


namespace Engine
{
    class SkyBox
    {
    private:
        Shader m_Shader;
        uint32_t m_CubeMap = 0;
        
        Vertex* m_SkyBoxVertices = nullptr;
        uint32_t* m_SkyBoxIndices = nullptr;
        uint32_t m_SkyBoxVAO = -1;
        uint32_t m_SkyBoxVBO = -1;
        
        int m_Width = 0, m_Height = 0, m_Channels = 0;
        
        void LoadTexturesInternal(const std::string& texturesPath, const std::string& texturesFormat);
    public:
        SkyBox();
        SkyBox(const std::string& texturesPath, const std::string& texturesFormat = std::string{".png"});
        ~SkyBox();
        
        SkyBox& operator()(const std::string& texturesPath, const std::string& texturesFormat = std::string{".png"});
        
        void Draw();
        void SetVPMatrix(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        
        uint32_t GetSkyBoxCubeMap() const { return m_CubeMap;}
    };
}
