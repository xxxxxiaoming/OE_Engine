#pragma once
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Engine
{
    class Shader
    {
    private:
        std::unordered_map<std::string, int> m_UniformLocations;
        unsigned int m_ShaderID = 0;
        void CreateShaderInternal(const std::string& vsFile, const std::string& fsFile);
        unsigned int CompileShaderInternal(const char* source, unsigned int type);
    public:
        Shader();
        Shader(const std::string& vsFile = std::string{""}, const std::string& fsFile = std::string{""});
        ~Shader();
    
        void CreateShader(const std::string& vsFile = std::string{""}, const std::string& fsFile = std::string{""});
        
        void Use() const;
        void UnUse() const;
        void Delete();
        bool CheckShaderValidity() const { return m_ShaderID != 0; }
        
        void SetUniform1i(const std::string& name , const int value = 0);
        void SetUniform1f(const std::string& name, const float value = 0.0f);
        void SetUniform4f(const std::string& nam, const float value1 = 0.0f, const float value2 = 0.0f, const float value3 = 0.0f, const float value4 = 0.0f);
        void SetUniformMatrix4f(const std::string& name, const glm::mat4& value);
        
        void SetUniform1iv(const std::string& name, int count, const int* value);
        
        int GetUniformLocation(const std::string& name = std::string{""}) const;
    };
}