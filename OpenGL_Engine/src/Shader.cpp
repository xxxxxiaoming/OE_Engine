#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Helper.h"

Engine::Shader::Shader(const char* vsSource, const char* faSource, const char* gsShader)
{
    CreateShaderFromSourceInternal(vsSource, faSource, gsShader);
}

Engine::Shader::Shader(const std::string& vsFile, const std::string& fsFile, const std::string& gsFile)
{
    CreateShaderInternal(vsFile, fsFile, gsFile);
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

void Engine::Shader::CreateShader(const std::string& vsFile, const std::string& fsFile, const std::string& gsFile)
{
    CreateShaderInternal(vsFile, fsFile, gsFile);
}

void Engine::Shader::CreateShaderFromSource(const char* vsSource, const char* fsSource, const char* gsShader)
{
    CreateShaderFromSourceInternal(vsSource, fsSource, gsShader);
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

void Engine::Shader::CreateShaderFromSourceInternal(const char* vsSource, const char* fsSource, const char* gsSource)
{
    if (m_ShaderID == 0)
    {
        GLCALL(m_ShaderID = glCreateProgram());
    }
    
    uint32_t vsShaderID = 0;
    uint32_t fsShaderID = 0;
    uint32_t gsShaderID = 0;
    
    if (vsSource != nullptr)
    {
        vsShaderID = CompileShaderInternal(vsSource, GL_VERTEX_SHADER);
        
        if (vsShaderID != 0)
        {
            GLCALL(glAttachShader(m_ShaderID, vsShaderID));
        }
    }
    
    if (fsSource != nullptr)
    {
        fsShaderID = CompileShaderInternal(fsSource, GL_FRAGMENT_SHADER);
        
        if (fsShaderID != 0)
        {
            GLCALL(glAttachShader(m_ShaderID, fsShaderID));
        }
    }
    
    if (gsSource != nullptr)
    {
        gsShaderID = CompileShaderInternal(gsSource, GL_GEOMETRY_SHADER);
        
        if (gsShaderID !=0 )
        {
            GLCALL(glAttachShader(m_ShaderID, gsShaderID));
        }
    }
    
    if (vsShaderID != 0 || fsShaderID != 0 || gsShaderID != 0)
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

void Engine::Shader::CreateShaderInternal(const std::string& vsFile, const std::string& fsFile, const std::string& gsFile)
{
    if (m_ShaderID == 0)
    {
        GLCALL(m_ShaderID = glCreateProgram());
    }
    
    std::string vsSource{};
    std::string fsSource{};
    std::string gsSource{};
    unsigned int vsShaderID = 0;
    unsigned int fsShaderID = 0;
    unsigned int gsShaderID = 0;
    
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
    
    if (not gsFile.empty())
    {
        std::fstream gsFileStream{gsFile};
        
        if (gsFileStream)
        {
            std::stringstream ss;
            ss << gsFileStream.rdbuf();
            gsSource = ss.str();
            
            gsShaderID  = CompileShaderInternal(gsSource.c_str(), GL_GEOMETRY_SHADER);
            
            if(gsShaderID != 0)
            {
                GLCALL(glAttachShader(m_ShaderID, gsShaderID));
            }
        }
    }
 
    if (vsShaderID != 0 || fsShaderID != 0 || gsShaderID != 0)
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
            
            if (gsShaderID != 0)
            {
                GLCALL(glDeleteShader(gsShaderID));
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
        GLCALL(glProgramUniform1i(m_ShaderID, loc, value));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniform1i(m_ShaderID, loc, value));
        }
    }
}

void Engine::Shader::SetUniform1f(const std::string& name, const float value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniform1f(m_ShaderID, loc, value));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniform1f(m_ShaderID, loc, value));
        }
    }
}

void Engine::Shader::SetUniform3f(const std::string& name, const float value1, const float value2, const float value3)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniform3f(m_ShaderID, loc, value1, value2, value3));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniform3f(m_ShaderID, loc, value1, value2, value3));
        }
    }
}

void Engine::Shader::SetUniform4f(const std::string& name, const float value1, const float value2, const float value3, const float value4)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniform4f(m_ShaderID, loc, value1, value2, value3, value4));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniform4f(m_ShaderID, loc, value1, value2, value3, value4));
        }
    }
}


void Engine::Shader::SetUniformMatrix3f(const std::string& name, const glm::mat3& value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniformMatrix3fv(m_ShaderID, loc, 1, GL_FALSE, &value[0][0]));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniformMatrix3fv(m_ShaderID, loc, 1, GL_FALSE, &value[0][0]));
        }
    }   
}

void Engine::Shader::SetUniformMatrix4f(const std::string& name, const glm::mat4& value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniformMatrix4fv(m_ShaderID, loc, 1, GL_FALSE, &value[0][0]));
    }
    else if (!name.empty())
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniformMatrix4fv(m_ShaderID, loc, 1, GL_FALSE, &value[0][0]));
        }
    }   
}

void Engine::Shader::SetUniform1iv(const std::string& name, int count, const int* value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniform1iv(m_ShaderID, loc, count, value));
    }
    else
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniform1iv(m_ShaderID, loc, count, value));
        }
    }
}

void Engine::Shader::SetUniform1fv(const std::string& name, int count, const float* value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniform1fv(m_ShaderID, loc, count, value));
    }
    else
    {
        int loc = GetUniformLocation(name);
        //ASSERT(loc != -1);

        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniform1fv(m_ShaderID, loc, count, value));
        }
    }
}

void Engine::Shader::SetUniformMatrix4fv(const std::string& name, int count, const glm::mat4* value)
{
    if (m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        int loc = m_UniformLocations[name];
        GLCALL(glProgramUniformMatrix4fv(m_ShaderID, loc, count, GL_FALSE, glm::value_ptr(value[0][0])));
    }
    else
    {
        int loc = GetUniformLocation(name);
        
        if (loc != -1)
        {
            m_UniformLocations[name] = loc;
            GLCALL(glProgramUniformMatrix4fv(m_ShaderID, loc, count, GL_FALSE, glm::value_ptr(value[0])));
        }
    }
}

int Engine::Shader::GetUniformLocation(const std::string& name) const
{
    if (not name.empty())
    {
        GLCALL(int loc = glGetUniformLocation(m_ShaderID, name.c_str()));
        //ASSERT(loc != -1);
        
        return loc;
    }
    else
        return -1;
}
