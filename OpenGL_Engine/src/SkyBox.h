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
        uint32_t m_CubeMap;
        
        Vertex* m_SkyBoxVertices;
        uint32_t* m_SkyBoxIndices;
        uint32_t m_SkyBoxVAO;
        uint32_t m_SkyBoxVBO;
        
        int m_Width, m_Height, m_Channels;
        
        void LoadTexturesInternal(const std::string& texturesPath, const std::string& texturesFormat);
    public:
        SkyBox(const std::string& texturesPath, const std::string& texturesFormat = std::string{".png"});
        ~SkyBox();
        
        void Draw();
        void SetVPMatrix(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        
        uint32_t GetSkyBoxCubeMap() const { return m_CubeMap;}
    };
}
