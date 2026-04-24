#pragma once

#define ASSERT(x) if(!(x)) __debugbreak() /* (x)记得加括号！！！ */
#define GLCALL(x) glClearError(); x; ASSERT(glLogCall(#x, __FILE__, __LINE__))

/* debug */
void glClearError();
bool glLogCall(const char* function, const char* file, int line);

/* shader */
// unsigned int CreateShader(const std::string& source, unsigned int type);
// unsigned int CreateProgram(const std::string& vs, const std::string& fs);
// unsigned int CreateProgramWithShaderFile(const std::string& vsFile, const std::string& fsFile);
