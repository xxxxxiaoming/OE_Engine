#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "Helper.h"
void glClearError()
{
	while (glGetError()) {}
}

bool glLogCall(const char* function, const char* file, int line)
{
	while (GLenum errorCode = glGetError())
	{
		printf("GL Error Code: %u \n %s %s:%d\n", errorCode, function , file, line);
		return false;
	}

	return true;
}

unsigned int CreateShader(const std::string& source, unsigned int type)
{
	unsigned int shaderId = glCreateShader(type);
	const char* srcStrPointer = source.c_str();
	GLCALL(glShaderSource(shaderId, 1, &srcStrPointer, NULL));
	GLCALL(glCompileShader(shaderId));

	// Check if compiling successfully.
	int compileStatus;
	GLCALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus));
	if (compileStatus != GL_TRUE)
	{
		// Get error log.
		int logLength;
		GLCALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength));

		char* logStrPtr = (char*)alloca(sizeof(char) * logLength);
		GLCALL(glGetShaderInfoLog(shaderId, sizeof(char) * logLength, &logLength, logStrPtr));

		printf("Shader compiling failed.\n Error Info: \n%s\n", logStrPtr);

		return 0;
	}
	
	return shaderId;
}

unsigned int CreateProgram(const std::string& vs, const std::string& fs)
{
	unsigned int programID = glCreateProgram();
	unsigned int vsShaderID = CreateShader(vs, GL_VERTEX_SHADER);
	unsigned int fsShaderID = CreateShader(fs, GL_FRAGMENT_SHADER);

	if (vsShaderID == 0 || fsShaderID == 0)
	{
		GLCALL(glDeleteProgram(programID));
		return 0;
	}

	GLCALL(glAttachShader(programID, vsShaderID));
	GLCALL(glAttachShader(programID, fsShaderID));
	GLCALL(glLinkProgram(programID));

	int linkStatus;
	GLCALL(glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus));

	if (linkStatus != GL_TRUE)
	{
		int logLength;
		GLCALL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength));
		
		char* logMessage = (char*)alloca(sizeof(char) * logLength);
		GLCALL(glGetProgramInfoLog(programID, sizeof(char) * logLength, &logLength, logMessage));
		
		GLCALL(glDeleteProgram(programID));

		printf("Shader linking failed. \n Error Info: \n%s", logMessage);

		return 0;
	}

	GLCALL(glDeleteShader(vsShaderID));
	GLCALL(glDeleteShader(fsShaderID));

	GLCALL(glValidateProgram(programID));

	return programID;
}

unsigned int CreateProgramWithShaderFile(const std::string& vsFile, const std::string& fsFile)
{
	std::fstream vsFStream{ vsFile };
	std::fstream fsFStream{ fsFile };

	if (!vsFStream || !fsFStream)
		return 0;

	std::string line;
	std::string vsSource;
	std::string fsSource;
	std::stringstream vsSourceStream;
	std::stringstream fsSourceStream;
	
	vsSourceStream << vsFStream.rdbuf();
	fsSourceStream << fsFStream.rdbuf();

	vsSource = vsSourceStream.str();
	fsSource = fsSourceStream.str();

	printf("Vertex Shader Source:\n\n%s\n\n", vsSource.c_str());
	printf("Fragment Shader Source:\n\n%s\n\n", fsSource.c_str());

	vsFStream.close();
	fsFStream.close();

	return CreateProgram(vsSource, fsSource);
}