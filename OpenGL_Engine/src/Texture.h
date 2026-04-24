#pragma once
#include <string>

namespace Engine
{
    class Texture
    {
    private:
        std::string m_FileName;
        unsigned char* m_TextLocalBuffer = nullptr;
        unsigned int m_GLTextureID = 0;
    
        int m_Width;
        int m_Height;
        int m_BPP;
    public:
        Texture(const std::string& fileName);
        ~Texture();
    
        void Delete();
        void Bind(int slot) const;
        void UnBind() const;
        
        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }
    };
}