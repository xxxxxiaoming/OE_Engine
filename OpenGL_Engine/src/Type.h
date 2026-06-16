#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace Engine
{
	enum class Operations
	{
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		MoveForward,
		MoveBackward,
		LookUp,
		LookDown,
		LookLeft,
		LookRight,
	};
	
	enum class BlendMode
	{
		Opaque,
		Masked,
		Transparent,
		TransparentMasked,
	};
	
	enum class PipeLine
	{
		PhongLight,
		PBR,
	};
	
	enum class TextureType
	{
		DIFFUSE, SPECULAR, NORMAL, NONE
	};
    struct vec2
    {
        float x, y;
    };

    struct vec3
    {
        float x, y, z;
    };

    struct vec4
    {
        float r, g, b, a;
    };

    struct Vertex
    {
        vec3 pos;
		vec3 normal;
        vec2 texCoord;
        vec4 color;
        float textureSlot;
    	
    	vec4 tangent;
    };
	
	struct Transform
	{
		glm::vec3 scale{1.0f, 1.0f, 1.0f};		// Scale of x, y, z
		glm::vec3 translation{0.0f, 0.0f, 0.0f};  // Translation of x, y, z
		glm::vec3 rotation{0.0f, 0.0f, 0.0f};		// rotation angle around x, y, z
	};
	
	struct MikkMeshData
	{
		std::vector<Vertex>* vertices;
		std::vector<uint32_t>* indices;
		uint8_t         indicesOfOneFace;
	};
	
	glm::mat4 GenerateModelMatrix(const Transform& transform);
}