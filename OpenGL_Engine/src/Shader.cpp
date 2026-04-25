#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "Shader.h"
#include "Helper.h"

Engine::Shader::Shader()
{
    CreateShader();
}

Engine::Shader::Shader(const std::string& vsFile, const std::string& fsFile)
{
    CreateShaderInternal(vsFile, fsFile);
}


Engine::Shader::~Shader()
{
    if (m_ShaderID != 0)
    {
        GLCALL(glDeleteProgram(m_ShaderID));
    }
}

void Engine::Shader::Use() const
{
    GLCALL(glUseProgram(m_ShaderID));
}

void Engine::Shader::UnUse() const
{
    GLCALL(glUseProgram(0));
}

void Engine::Shader::Delete()
{
    if (m_ShaderID != 0)
    {
        GLCALL(glDeleteProgram(m_ShaderID));
        m_ShaderID = 0;
    }
}

void Engine::Shader::CreateShader(const std::string& vsFile, const std::string& fsFile)
{
    CreateShaderInternal(vsFile, fsFile);
}

unsigned int Engine::Shader::CompileShaderInternal(const char* source, const unsigned int type)
{
    GLCALL(unsigned int shaderID = glCreateShader(type));
    
    if (shaderID == 0)
        return 0;
    
    GLsizei length = 0;
    /* TODO : Add comments for every parameter's meaning of this function. */
    GLCALL(glShaderSource(shaderID, 1, &source, NULL));
    GLCALL(glCompileShader(shaderID));
    
    int shaderCompiled = 0;
    GLCALL(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled));
    
    if (shaderCompiled == GL_FALSE)
    {
        // Get log content length.(How many characters in log string)
        GLCALL(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length));
        
        // Allocating memory for log string
        char* log = static_cast<char*>(alloca(sizeof(char) * length));
        GLCALL(glGetShaderInfoLog(shaderID, sizeof(char) * length, &length, log));
        GLCALL(glDeleteShader(shaderID));
        
        printf("Shader compiling failed.\n Error Info: \n%s\n", log);
        
        shaderID = 0;
    }
    
    return shaderID;
}

void Engine::Shader::CreateShaderInternal(const std::string& vsFile = std::string{""}, const std::string& fsFile = std::string{""})
{
    if (m_ShaderID == 0)
    {
        GLCALL(m_ShaderID = glCreateProgram());
    }
    
    std::string vsSource{};
    std::string fsSource{};
    unsigned int vsShaderID = 0;
    unsigned int fsShaderID = 0;
    
    if (not vsFile.empty())
    {
        /* Read vertex shader file content */
        std::fstream vsFileStream{vsFile};
        
        if (vsFileStream)
        {
            std::stringstream ss;
            ss << vsFileStream.rdbuf();
            vsSource = ss.str();
            
            vsShaderID  = CompileShaderInternal(vsSource.c_str(), GL_VERTEX_SHADER);
            
            if (vsShaderID != 0)
            {
                GLCALL(glAttachShader(m_ShaderID, vsShaderID));
            }
        }
    }
    
    if (not fsFile.empty())
    {
        /* Read fragment shader file content */
        std::fstream fsFileStream{fsFile};
        
        if (fsFileStream)
        {
            std::stringstream ss;
            ss << fsFileStream.rdbuf();
            fsSource = ss.str();
            
            fsShaderID = CompileShaderInternal(fsSource.c_str(), GL_FRAGMENT_SHADER);
            
            if (fsShaderID != 0)
            {
                GLCALL(glAttachShader(m_ShaderID, fsShaderID));
            }
        }
    }
 
    if (vsShaderID != 0 || fsShaderID != 0)
    {
        GLCALL(glLinkProgram(m_ShaderID));
        
        int linkStatus = 0;
        GLCALL(glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &linkStatus));
        
        if (linkStatus == GL_FALSE)
        {
            int length = 0;
            GLCALL(glGetProgramiv(m_ShaderID, GL_INFO_LOG_LENGTH, &length));
            
            char* log = static_cast<char*>(alloca(sizeof(char) * length));
            GLCALL(glGetProgramInfoLog(m_ShaderID, sizeof(char) * length, &length, log));
            GLCALL(glDeleteProgram(m_ShaderID));
            
            m_ShaderID = 0;
            printf("Shader linking failed. \n Error Info: \n%s", log);
        }
        else
        {
            if (vsShaderID != 0)
            {
                GLCALL(glDeleteShader(vsShaderID));
            }
            
            if (fsShaderID != 0)
            {
                GLCALL(glDeleteShader(fsShaderID));
            }
            
            GLCALL(glValidateProgram(m_ShaderID));
        }
    }
}

void Engine::Shader::SetUniform1i(const std::string& name, const int value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniform1i(loc, value));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniform1i(loc, value));
        }
    }
}

void Engine::Shader::SetUniform1f(const std::string& name, const float value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniform1f(loc, value));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniform1f(loc, value));
        }
    }
}

void Engine::Shader::SetUniform3f(const std::string& name, const float value1, const float value2, const float value3)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniform3f(loc, value1, value2, value3));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniform3f(loc, value1, value2, value3));
        }
    }
}

void Engine::Shader::SetUniform4f(const std::string& name, const float value1, const float value2, const float value3, const float value4)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniform4f(loc, value1, value2, value3, value4));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniform3f(loc, value1, value2, value3));
        }
    }
}


void Engine::Shader::SetUniformMatrix3f(const std::string& name, const glm::mat3& value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]));
        }
    }   
}

void Engine::Shader::SetUniformMatrix4f(const std::string& name, const glm::mat4& value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]));
        }
    }   
}

void Engine::Shader::SetUniform1iv(const std::string& name, int count, const int* value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniform1iv(loc, count, value));
    }
    else
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniform1iv(loc, count, value));
        }
    }
}

void Engine::Shader::SetUniform1fv(const std::string& name, int count, const float* value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glUniform1fv(loc, count, value));
    }
    else
    {
        int loc = GetUniformLocation(name);
        ASSERT(loc != -1);

        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glUniform1fv(loc, count, value));
        }
    }
}

int Engine::Shader::GetUniformLocation(const std::string& name) const
{
    if (not name.empty())
    {
        GLCALL(int loc = glGetUniformLocation(m_ShaderID, name.c_str()));
        ASSERT(loc != -1);
        
        return loc;
    }
    else
        return -1;
}
