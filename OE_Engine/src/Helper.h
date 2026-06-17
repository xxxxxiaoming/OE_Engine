#pragma once
#include "Type.h"
#include <string>

#define ASSERT(x) if(!(x)) __debugbreak() /* (x)记得加括号！！！ */
#define GLCALL(x) glClearError(); x; ASSERT(glLogCall(#x, __FILE__, __LINE__))

/* debug */
void glClearError();
bool glLogCall(const char* function, const char* file, int line);

namespace Engine
{
    void CalcMikkTSpaceTangents(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint8_t indicesPerFace);
    
    void createRectangle(const vec3* positions, const float* width, const float* height, Vertex* vertices, uint32_t* indices, int SIZE);
    void CreateCube(const vec3* centers, const float* width, const float* height, const float* depth, Vertex* vertices, uint32_t* indices, int SIZE);
    void CreateSphere(const vec3* centers, const float* radius, int sectors, int stacks, Vertex* vertices, uint32_t* indices, int SIZE);
    void PBRTexturePhaser(std::string& albedo, std::string& metallic, std::string& roughness, std::string& ao, std::string& normal, const std::string& assetPath, const std::string& assetName, const std::string& normalFormat);
}

/* shader */
// unsigned int CreateShader(const std::string& source, unsigned int type);
// unsigned int CreateProgram(const std::string& vs, const std::string& fs);
// unsigned int CreateProgramWithShaderFile(const std::string& vsFile, const std::string& fsFile);