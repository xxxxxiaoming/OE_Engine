#include <glad/glad.h>
#include <stb_image.h>
#include "SkyBox.h"
#include "Helper.h"

// language=GLSL
const char vsSource[] = R"glsl(
#version 330 core

layout (location = 0) in vec3 vertexPosition;

out vec3 textureDir;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    vec4 position = u_Projection * u_View * vec4(vertexPosition, 1.0);

    textureDir = vertexPosition;
    gl_Position = vec4(position.x, position.y, position.w, position.w);
}
)glsl";

// language=GLSL
const char fsSource[] = R"glsl(
#version 330 core

in vec3 textureDir; // 代表3D纹理坐标的方向向量

out vec4 color;

uniform samplerCube u_CubeMap; // 立方体贴图的纹理采样器

void main()
{             
    color = texture(u_CubeMap, textureDir);
}
)glsl";

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

Engine::SkyBox::SkyBox(const std::string& texturesPath, const std::string& texturesFroramt) : m_Shader(vsSource, fsSource)
{
    LoadTexturesInternal(texturesPath, texturesFroramt);
    
    GLCALL(glGenVertexArrays(1, &m_SkyBoxVAO));
    GLCALL(glGenBuffers(1, &m_SkyBoxVBO));
    GLCALL(glBindVertexArray(m_SkyBoxVAO));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_SkyBoxVBO));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW));
    GLCALL(glEnableVertexAttribArray(0));
    GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCALL(glBindVertexArray(0));
}

Engine::SkyBox::~SkyBox()
{
    if (m_CubeMap == 0)
        return;
    
    GLCALL(glDeleteTextures(1, &m_CubeMap));
}

void Engine::SkyBox::LoadTexturesInternal(const std::string& texturesPath, const std::string& texturesFormat)
{
    GLCALL(glGenTextures(1, &m_CubeMap));
    GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap));
    
    const std::string pngName[6] = {
        "right",
        "left",
        "top",
        "bottom",
        "front",
        "back"
    };
    
    for (int index = 0; index < 6; index++)
    {
        std::string fullpath = texturesPath + pngName[index] + texturesFormat;
        unsigned char* textureBuffer = stbi_load(fullpath.c_str(), &m_Width, &m_Height, &m_Channels, 3);
        
        GLCALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureBuffer));
        stbi_image_free(textureBuffer);
    }
    
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ));
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ));
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE ));
    
    GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));    
}

void Engine::SkyBox::Draw()
{
    m_Shader.Use();
    m_Shader.SetUniform1i("u_CubeMap", 0);
    
    GLCALL(glDepthFunc(GL_LEQUAL));
    GLCALL(glBindVertexArray(m_SkyBoxVAO));
    GLCALL(glActiveTexture(GL_TEXTURE0));
    GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap));
    GLCALL(glDrawArrays(GL_TRIANGLES, 0, 36));
    GLCALL(glDepthFunc(GL_LESS));
    GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    
    m_Shader.UnUse();
}

void Engine::SkyBox::SetVPMatrix(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    
    m_Shader.SetUniformMatrix4f("u_View", viewMatrix);
    m_Shader.SetUniformMatrix4f("u_Projection", projectionMatrix);
    
    m_Shader.UnUse();
}
