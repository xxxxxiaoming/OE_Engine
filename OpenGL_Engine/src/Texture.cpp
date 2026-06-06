#include <glad/glad.h>
#include <stb_image.h>
#include <cstdio>
#include "Texture.h"
#include "Helper.h"

Engine::Texture::Texture() :
	m_FileName(std::string{ "" }), m_GLTextureID(0), m_Width(0), m_Height(0), m_BPP(0)
{
}

Engine::Texture::Texture(uint32_t colorData) : m_FileName("Memory_Texture"), m_Width(1), m_Height(1), m_BPP(4)
{
    GLCALL(glGenTextures(1, &m_GLTextureID));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_GLTextureID));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    // 直接把这 4 个字节当成像素数据传给 OpenGL
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colorData));
}

Engine::Texture::Texture(const std::string& fileName, bool bFlipVertical) : m_FileName(fileName)
{
    if (CACHE_MAP.find(fileName) != CACHE_MAP.end() && CACHE_MAP[fileName] != nullptr)
    {
        // TODO: Multi thread safety & need improve
        const Texture* texture = CACHE_MAP[fileName];
        
        m_GLTextureID = texture->m_GLTextureID;
        m_Width = texture->m_Width;
        m_Height = texture->m_Height;
        m_BPP = texture->m_BPP;
        
        REF_MAP[fileName]++;
        
        return;
    }
    
    /* 垂直翻转图片，使图片的坐标原点在左下角 */
    if (bFlipVertical)
        stbi_set_flip_vertically_on_load(1);
    
    /* load texture image */
    unsigned char* textLocalBuffer = stbi_load(fileName.c_str(), &m_Width, &m_Height,&m_BPP, 0);

    int innerFormat = GL_RGBA8;
    int format = GL_RGBA;

    if (m_BPP == 1)
    {
        innerFormat = GL_RED;
        format = GL_RED;
    }
    else if (m_BPP == 3)
    {
        innerFormat = GL_RGB8;
        format = GL_RGB;
    }
    else if (m_BPP == 4)
    {
        innerFormat = GL_RGBA8;
        format = GL_RGBA;
    }
    
    /* Create texture buffer and bind buffer to operate */
    GLCALL(glGenTextures(1, &m_GLTextureID));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_GLTextureID));
    
    /* Set texture parameters (4 parameters commonly) */
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));   /* Tell OpenGL how to down sample (when viewport is not fit with size of texture) */
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));   /* Tell OpenGL how to up sample (when viewport is not fit with size of texture)*/
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));/* 设置纹理的横向填充方式（纹理的横向比例与viewport比例不适配），GL_CLAMP_TO_EDGE 平铺 */
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));/* 设置纹理的竖向填充方式（纹理的竖向比例与viewport比例不适配），GL_CLAMP_TO_EDGE 平铺 */
    
    /* Send texture data to texture buffer */
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, innerFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, textLocalBuffer));
    GLCALL(glGenerateMipmap(GL_TEXTURE_2D));

    // TODO: Multi thread safety
    CACHE_MAP[fileName] = this;
    REF_MAP[fileName] = 1;
    
    if (textLocalBuffer != nullptr)
        stbi_image_free(textLocalBuffer);
}

Engine::Texture::Texture(const Texture& texture) noexcept:
    m_FileName(texture.m_FileName), m_GLTextureID(texture.m_GLTextureID), m_Width(texture.m_Width), m_Height(texture.m_Height), m_BPP(texture.m_BPP), m_Slot(texture.m_Slot)
{ }

Engine::Texture::Texture(Texture&& texture) noexcept:
    m_FileName(texture.m_FileName), m_GLTextureID(texture.m_GLTextureID), m_Width(texture.m_Width), m_Height(texture.m_Height), m_BPP(texture.m_BPP), m_Slot(texture.m_Slot)
{
    texture.m_FileName = "";
    texture.m_GLTextureID = 0;
    texture.m_Width = 0;
    texture.m_Height = 0;
    texture.m_BPP = 0;
    texture.m_Slot = 0;
}

Engine::Texture::~Texture()
{
    Delete();
}

Engine::Texture& Engine::Texture::operator=(const Texture& texture) noexcept
{
    m_FileName = texture.m_FileName;
    m_GLTextureID = texture.m_GLTextureID;
    m_Width = texture.m_Width;
    m_Height = texture.m_Height;
    m_BPP = texture.m_BPP;
    m_Slot = texture.m_Slot;

    return *this;
}

Engine::Texture& Engine::Texture::operator=(Texture&& texture) noexcept
{
    m_FileName = texture.m_FileName;
    m_GLTextureID = texture.m_GLTextureID;
    m_Width = texture.m_Width;
    m_Height = texture.m_Height;
    m_BPP = texture.m_BPP;
    m_Slot = texture.m_Slot;

    texture.m_FileName = "";
    texture.m_GLTextureID = 0;
    texture.m_Width = 0;
    texture.m_Height = 0;
    texture.m_BPP = 0;
    texture.m_Slot = 0;

    return *this;
}

void Engine::Texture::Delete()
{
    if (m_GLTextureID != 0)
    {
        REF_MAP[m_FileName]--;
        if (REF_MAP[m_FileName] <= 0)
        {
            GLCALL(glDeleteTextures(1, &m_GLTextureID));
            
            // TODO: Multi thread safety.
            CACHE_MAP[m_FileName] = nullptr;
            m_GLTextureID = 0;
        }
    }
}

void Engine::Texture::Bind(int slot)
{
    /* OpenGL 支持处理多张纹理，通过给不同纹理插槽绑定不同的纹理缓冲区，下面两句代码需要成对出现，意味着，将m_TextureID这个纹理缓冲区绑定到GL_TEXTURE0 + slot 这个纹理插槽上去 */
    m_Slot = slot;
    // GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    // GLCALL(glBindTexture(GL_TEXTURE_2D, m_GLTextureID));
    /* 现代OpenGL(OpenGL 4.5及之后)提供了有一个合二为一的API */
    GLCALL(glBindTextureUnit(slot, m_GLTextureID));
}

void Engine::Texture::UnBind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void Engine::Texture::PrintTextureLimitInfo()
{
    int maxVertexTextureImageUnits = 0;
	int maxFragmentTextureImageUnits = 0;
	int maxCombinedTextureImageUnits = 0;

	GLCALL(glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertexTextureImageUnits));
	GLCALL(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxFragmentTextureImageUnits));
	GLCALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits));

    printf("Max vertex shader texture limit: %d", maxVertexTextureImageUnits);
	printf("Max fragment shader texture limit: %d", maxFragmentTextureImageUnits);
	printf("Max combined texture limit: %d", maxCombinedTextureImageUnits);
}
