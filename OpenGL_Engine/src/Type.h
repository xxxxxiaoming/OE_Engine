#pragma once

#include <array>

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
    };

    template <size_t SIZE = 1>
	void createRectangle(const vec3* positions, const float* width, const float* height, Vertex* vertices, int* indices)
	{
		for (int count = 0; count < SIZE; count++)
		{
			const vec3& position = positions[count];
			const float& w = width[count];
			const float& h = height[count];

			Vertex& vertexLeftBottom = vertices[0 + count * 4];
			Vertex& vertexRightBottom = vertices[1 + count * 4];
			Vertex& vertexRightTop = vertices[2 + count * 4];
			Vertex& vertexLeftTop = vertices[3 + count * 4];

			vertexLeftBottom.pos = { position.x, position.y, position.z };
			vertexLeftBottom.normal = { 0.0f, 0.0f, 1.0f };
			vertexLeftBottom.texCoord = { 0.0f, 0.0f };
			vertexLeftBottom.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexLeftBottom.textureSlot = 0.0f;

			vertexRightBottom.pos = { position.x + w, position.y, position.z };
			vertexRightBottom.normal = { 0.0f, 0.0f, 1.0f };
			vertexRightBottom.texCoord = { 1.0f, 0.0f };
			vertexRightBottom.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexRightBottom.textureSlot = 0.0f;

			vertexRightTop.pos = { position.x + w, position.y + h, position.z };
			vertexRightTop.normal = { 0.0f, 0.0f, 1.0f };
			vertexRightTop.texCoord = { 1.0f, 1.0f };
			vertexRightTop.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexRightTop.textureSlot = 0.0f;

			vertexLeftTop.pos = { position.x, position.y + h, position.z };
			vertexLeftTop.normal = { 0.0f, 0.0f, 1.0f };
			vertexLeftTop.texCoord = { 0.0f, 1.0f };
			vertexLeftTop.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexLeftTop.textureSlot = 0.0f;
		}

		for (int count = 0; count < SIZE; count++)
		{
			indices[0 + count * 6] = 0 + 4 * count;
			indices[1 + count * 6] = 1 + 4 * count;
			indices[2 + count * 6] = 2 + 4 * count;
			indices[3 + count * 6] = 2 + 4 * count;
			indices[4 + count * 6] = 3 + 4 * count;
			indices[5 + count * 6] = 0 + 4 * count;
		}
	}
	
	template<size_t SIZE>
	void CreateCube(const vec3* centers, const float* width, const float* height, const float* depth, Vertex* vertices, int* indices)
	{
		for (int count = 0; count < SIZE; count++)
		{
			const vec3& center = centers[count];
			const float wH = width[count] / 2.0f;  // width Half
			const float hH = height[count] / 2.0f; // height Half
			const float dH = depth[count] / 2.0f;  // depth Half

			// 偏移量：每个立方体占用 24 个顶点
			int vOffset = count * 24;
			int iOffset = count * 36;

			// --- 1. 前面 (Front Face, +Z) ---
			vertices[vOffset + 0] = { {center.x - wH, center.y - hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 1] = { {center.x + wH, center.y - hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 2] = { {center.x + wH, center.y + hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 3] = { {center.x - wH, center.y + hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };

			// --- 2. 后面 (Back Face, -Z) ---
			vertices[vOffset + 4] = { {center.x - wH, center.y - hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 5] = { {center.x - wH, center.y + hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 6] = { {center.x + wH, center.y + hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 7] = { {center.x + wH, center.y - hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };

			// --- 3. 左面 (Left Face, -X) ---
			vertices[vOffset + 8] = { {center.x - wH, center.y - hH, center.z - dH}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 9]  = { {center.x - wH, center.y - hH, center.z + dH}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 10] = { {center.x - wH, center.y + hH, center.z + dH}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 11] = { {center.x - wH, center.y + hH, center.z - dH}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };

			// --- 4. 右面 (Right Face, +X) ---
			vertices[vOffset + 12] = { {center.x + wH, center.y - hH, center.z + dH}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 13] = { {center.x + wH, center.y - hH, center.z - dH}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 14] = { {center.x + wH, center.y + hH, center.z - dH}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 15] = { {center.x + wH, center.y + hH, center.z + dH}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };

			// --- 5. 上面 (Top Face, +Y) ---
			vertices[vOffset + 16] = { {center.x - wH, center.y + hH, center.z + dH}, {0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f }, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 17] = { {center.x + wH, center.y + hH, center.z + dH}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 18] = { {center.x + wH, center.y + hH, center.z - dH}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 19] = { {center.x - wH, center.y + hH, center.z - dH}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };

			// --- 6. 下面 (Bottom Face, -Y) ---
			vertices[vOffset + 20] = { {center.x - wH, center.y - hH, center.z - dH}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 21] = { {center.x + wH, center.y - hH, center.z - dH}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 22] = { {center.x + wH, center.y - hH, center.z + dH}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };
			vertices[vOffset + 23] = { {center.x - wH, center.y - hH, center.z + dH}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f };

			// --- 设置 12 个三角形的索引 (每个面 2 个) ---
			for (int face = 0; face < 6; face++)
			{
				int baseV = vOffset + face * 4;
				int baseI = iOffset + face * 6;
				indices[baseI + 0] = baseV + 0;
				indices[baseI + 1] = baseV + 1;
				indices[baseI + 2] = baseV + 2;
				indices[baseI + 3] = baseV + 2;
				indices[baseI + 4] = baseV + 3;
				indices[baseI + 5] = baseV + 0;
			}
		}
	}
}