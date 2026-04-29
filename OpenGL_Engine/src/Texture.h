#pragma once
#include <string>
#include <unordered_map>
#include "Type.h"

namespace Engine
{
    class Texture
    {
    private:
        std::string m_FileName;
        unsigned int m_GLTextureID = 0;
    
        int m_Width;
        int m_Height;
        int m_BPP;

        inline static std::unordered_map<std::string, const Texture*> CACHE_MAP;
        inline static std::unordered_map<std::string, uint32_t> REF_MAP;

    public:
        int m_Slot = 0;

        Texture();
        Texture(const std::string& fileName);
        ~Texture();

        static void PrintTextureLimitInfo();
    
        void Delete();
        void Bind(int slot);
        void UnBind() const;
        
		inline bool CheckTextureValidity() const { return m_GLTextureID != 0; }

		inline int GetTextureSlot() const { return m_Slot; }
        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }
    };
}