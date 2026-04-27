#include "Material.h"

Engine::Material::Material(const std::string& vsPath, const std::string& fsPath, const std::string* textures, const int* textureSlots, int textureNum)
    : m_Shader{ vsPath, fsPath }
{
	textureNum > MAX_TEXTURES ? textureNum = MAX_TEXTURES : textureNum;
    m_Texture.reserve(textureNum);
    for (int index = 0; index < textureNum; index++)
    {
        m_Texture.emplace_back(textures[index]);
		m_Texture.back().m_Slot = textureSlots[index];
    }
}

Engine::Material::~Material()
{
    if(!m_Cleared)
	    Delete();
}

void Engine::Material::UseMaterial()
{
	m_Shader.Use();
    if (m_Texture.size() > 0)
    {
        for(auto& texture : m_Texture)
        {
            texture.Bind(texture.GetTextureSlot());
		}
    }
}

void Engine::Material::UnuseMaterial()
{
    m_Shader.UnUse();
}

void Engine::Material::Delete()
{
    if (m_Cleared)
        return;

    if (m_Shader.CheckShaderValidity())
    {
        m_Shader.Delete();
    }
    
    for (auto& texture : m_Texture)
    {
        if(texture.CheckTextureValidity())
            texture.Delete();
    }

    m_Cleared = true;
}
