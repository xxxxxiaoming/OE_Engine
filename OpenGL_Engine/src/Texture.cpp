#include <glad/glad.h>
#include <stb_image.h>
#include "Texture.h"
#include "Helper.h"

Engine::Texture::Texture(const std::string& fileName) : m_FileName(fileName)
{
    /* 垂直翻转图片，使图片的坐标原点在左下角 */
    stbi_set_flip_vertically_on_load(1);
    /* load texture image */
    m_TextLocalBuffer = stbi_load(fileName.c_str(), &m_Width, &m_Height,&m_BPP, 4);
    
    /* Create texture buffer and bind buffer to operate */
    GLCALL(glGenTextures(1, &m_GLTextureID));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_GLTextureID));
    
    /* Set texture parameters (4 parameters commonly) */
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));   /* Tell OpenGL how to down sample (when viewport is not fit with size of texture) */
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));   /* Tell OpenGL how to up sample (when viewport is not fit with size of texture)*/
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));/* 设置纹理的横向填充方式（纹理的横向比例与viewport比例不适配），GL_CLAMP_TO_EDGE 平铺 */
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));/* 设置纹理的竖向填充方式（纹理的竖向比例与viewport比例不适配），GL_CLAMP_TO_EDGE 平铺 */
    
    /* Send texture data to texture buffer */
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_TextLocalBuffer));
    GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
    
    if (m_TextLocalBuffer != nullptr)
        stbi_image_free(m_TextLocalBuffer);
}

Engine::Texture::~Texture()
{
    if (m_GLTextureID != 0)
    {
        GLCALL(glDeleteTextures(1, &m_GLTextureID));
    }
}

void Engine::Texture::Delete()
{
    if (m_GLTextureID != 0)
    {
        GLCALL(glDeleteTextures(1, &m_GLTextureID));
        m_GLTextureID = 0;
    }
}

void Engine::Texture::Bind(int slot) const
{
    /* OpenGL 支持处理多张纹理，通过给不同纹理插槽绑定不同的纹理缓冲区，下面两句代码需要成对出现，意味着，将m_TextureID这个纹理缓冲区绑定到GL_TEXTURE0 + slot 这个纹理插槽上去 */
    GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_GLTextureID));
    /* 现代OpenGL(OpenGL 4.5及之后)提供了有一个合二为一的API */
    // GLCALL(glBindTextureUnit(slot, m_GLTextureID));
}

void Engine::Texture::UnBind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}
