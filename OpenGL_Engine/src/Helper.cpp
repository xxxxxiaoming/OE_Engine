#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <vector>

#include "Helper.h"
#include <mikktspace.h>

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

namespace Engine
{
	void CalcMikkTSpaceTangents(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint8_t indicesPerFace)
	{
		MikkMeshData meshData{ &vertices, &indices, indicesPerFace };

		SMikkTSpaceInterface iface{};

		iface.m_getNumFaces = [](const SMikkTSpaceContext* ctx) -> int {
			auto* data = static_cast<Engine::MikkMeshData*>(ctx->m_pUserData);
			return (int)data->indices->size() / data->indicesOfOneFace;
		};

		iface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* ctx, const int) -> int {
			auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
			return data->indicesOfOneFace;
		};

		iface.m_getPosition = [](const SMikkTSpaceContext* ctx, float out[], const int iFace, const int iVert) {
			auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
			const auto& pos = (*data->vertices)[(*data->indices)[iFace * data->indicesOfOneFace + iVert]].pos;
			out[0] = pos.x; out[1] = pos.y; out[2] = pos.z;
		};

		iface.m_getNormal = [](const SMikkTSpaceContext* ctx, float out[], const int iFace, const int iVert) {
			auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
			const auto& n = (*data->vertices)[(*data->indices)[iFace * data->indicesOfOneFace + iVert]].normal;
			out[0] = n.x; out[1] = n.y; out[2] = n.z;
		};

		iface.m_getTexCoord = [](const SMikkTSpaceContext* ctx, float out[], const int iFace, const int iVert) {
			auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
			const auto& uv = (*data->vertices)[(*data->indices)[iFace * data->indicesOfOneFace + iVert]].texCoord;
			out[0] = uv.x; out[1] = uv.y;
		};

		iface.m_setTSpaceBasic = [](const SMikkTSpaceContext* ctx, const float tangent[], const float fSign, const int iFace, const int iVert) {
			auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
			
			uint32_t indexInIndexArray = iFace * data->indicesOfOneFace + iVert;
			uint32_t origVertIdx = (*data->indices)[indexInIndexArray];
			
			Vertex currVert = (*data->vertices)[origVertIdx]; 
			vec4 newTangent = { tangent[0], tangent[1], tangent[2], fSign };

			bool isUninitialized = (currVert.tangent.r == 0.0f && currVert.tangent.g == 0.0f && currVert.tangent.b == 0.0f);

			if (isUninitialized) 
			{
				(*data->vertices)[origVertIdx].tangent = newTangent;
			} 
			else 
			{
				float dot = currVert.tangent.r * newTangent.r + 
							currVert.tangent.g * newTangent.g + 
							currVert.tangent.b * newTangent.b;
				
				if (dot < 0.99f || currVert.tangent.a != newTangent.a) 
				{
					currVert.tangent = newTangent;
					data->vertices->push_back(currVert);
					(*data->indices)[indexInIndexArray] = static_cast<uint32_t>(data->vertices->size() - 1);
				}
			}
		};

		SMikkTSpaceContext ctx{};
		ctx.m_pInterface = &iface;
		ctx.m_pUserData  = &meshData;

		genTangSpaceDefault(&ctx);
	}
	
	void createRectangle(const vec3* positions, const float* width, const float* height, Vertex* vertices, uint32_t* indices, int SIZE)
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
			vertexLeftBottom.tangent = { 0.0f, 0.0f, 0.0f, 0.0f };

			vertexRightBottom.pos = { position.x + w, position.y, position.z };
			vertexRightBottom.normal = { 0.0f, 0.0f, 1.0f };
			vertexRightBottom.texCoord = { 1.0f, 0.0f };
			vertexRightBottom.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexRightBottom.textureSlot = 0.0f;
			vertexRightBottom.tangent = { 0.0f, 0.0f, 0.0f, 0.0f };

			vertexRightTop.pos = { position.x + w, position.y + h, position.z };
			vertexRightTop.normal = { 0.0f, 0.0f, 1.0f };
			vertexRightTop.texCoord = { 1.0f, 1.0f };
			vertexRightTop.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexRightTop.textureSlot = 0.0f;
			vertexRightTop.tangent = { 0.0f, 0.0f, 0.0f, 0.0f };

			vertexLeftTop.pos = { position.x, position.y + h, position.z };
			vertexLeftTop.normal = { 0.0f, 0.0f, 1.0f };
			vertexLeftTop.texCoord = { 0.0f, 1.0f };
			vertexLeftTop.color = { 1.0f, 1.0f, 0.0f, 1.0f };
			vertexLeftTop.textureSlot = 0.0f;
			vertexLeftTop.tangent = { 0.0f, 0.0f, 0.0f, 0.0f };
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
		
		std::vector<Vertex> verticesVec(vertices, vertices + SIZE * 4);
		std::vector<uint32_t> indicesVec(indices, indices + SIZE * 6);
		
		CalcMikkTSpaceTangents(verticesVec, indicesVec, 3);
		
		std::memcpy(vertices, verticesVec.data(), verticesVec.size() * sizeof(Vertex));
		std::memcpy(indices, indicesVec.data(), indicesVec.size() * sizeof(uint32_t));
	}
	
	void CreateCube(const vec3* centers, const float* width, const float* height, const float* depth, Vertex* vertices, uint32_t* indices, int SIZE)
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
			vertices[vOffset + 0] = { {center.x - wH, center.y - hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 1] = { {center.x + wH, center.y - hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 2] = { {center.x + wH, center.y + hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 3] = { {center.x - wH, center.y + hH, center.z + dH}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };

			// --- 2. 后面 (Back Face, -Z) ---
			vertices[vOffset + 4] = { {center.x - wH, center.y - hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 5] = { {center.x - wH, center.y + hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 6] = { {center.x + wH, center.y + hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 7] = { {center.x + wH, center.y - hH, center.z - dH}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };

			// --- 3. 左面 (Left Face, -X) ---
			vertices[vOffset + 8] = { {center.x - wH, center.y - hH, center.z - dH}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 9]  = { {center.x - wH, center.y - hH, center.z + dH}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 10] = { {center.x - wH, center.y + hH, center.z + dH}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 11] = { {center.x - wH, center.y + hH, center.z - dH}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };

			// --- 4. 右面 (Right Face, +X) ---
			vertices[vOffset + 12] = { {center.x + wH, center.y - hH, center.z + dH}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 13] = { {center.x + wH, center.y - hH, center.z - dH}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 14] = { {center.x + wH, center.y + hH, center.z - dH}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 15] = { {center.x + wH, center.y + hH, center.z + dH}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };

			// --- 5. 上面 (Top Face, +Y) ---
			vertices[vOffset + 16] = { {center.x - wH, center.y + hH, center.z + dH}, {0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f }, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 17] = { {center.x + wH, center.y + hH, center.z + dH}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 18] = { {center.x + wH, center.y + hH, center.z - dH}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 19] = { {center.x - wH, center.y + hH, center.z - dH}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };

			// --- 6. 下面 (Bottom Face, -Y) ---
			vertices[vOffset + 20] = { {center.x - wH, center.y - hH, center.z - dH}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 21] = { {center.x + wH, center.y - hH, center.z - dH}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 22] = { {center.x + wH, center.y - hH, center.z + dH}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };
			vertices[vOffset + 23] = { {center.x - wH, center.y - hH, center.z + dH}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f} };

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
		
		std::vector<Vertex> verticesVec(vertices, vertices + SIZE * 24);
		std::vector<uint32_t> indicesVec(indices, indices + SIZE * 36);
		
		CalcMikkTSpaceTangents(verticesVec, indicesVec, 3);
		
		std::memcpy(vertices, verticesVec.data(), verticesVec.size() * sizeof(Vertex));
		std::memcpy(indices, indicesVec.data(), indicesVec.size() * sizeof(uint32_t));
	}
	
	void CreateSphere(const vec3* centers, const float* radius, int sectors, int stacks, Vertex* vertices, uint32_t* indices, int SIZE)
	{
	    const float PI_F = 3.14159265359f;
	    
	    // 计算单个球体所需的顶点数和索引数
	    int vPerSphere = (sectors + 1) * (stacks + 1);
	    int iPerSphere = stacks * sectors * 6;

	    for (int count = 0; count < SIZE; count++)
	    {
	        const vec3& center = centers[count];
	        const float& r = radius[count];

	        int vOffset = count * vPerSphere;
	        int iOffset = count * iPerSphere;

	        // --- 1. 生成球体顶点 ---
	        for (int i = 0; i <= stacks; ++i)
	        {
	            float phi = i * PI_F / stacks; // 从北极到南极 (0 到 PI)
	            float sinPhi = sin(phi);
	            float cosPhi = cos(phi);

	            for (int j = 0; j <= sectors; ++j)
	            {
	                float theta = j * 2.0f * PI_F / sectors; // 绕一圈 (0 到 2*PI)
	                float sinTheta = sin(theta);
	                float cosTheta = cos(theta);

	                int curV = vOffset + i * (sectors + 1) + j;

	                // 计算球面上相对原点的 3D 坐标方向 (以 Y 轴为正上方向，与你的 Cube 对齐)
	                float x = sinPhi * cosTheta;
	                float y = cosPhi;
	                float z = sinPhi * sinTheta;

	                // 填充顶点数据
	                vertices[curV].pos = { center.x + x * r, center.y + y * r, center.z + z * r };
	                vertices[curV].normal = { x, y, z }; // 球体表面法线就是其球心方向向量
	                vertices[curV].texCoord = { (float)j / sectors, 1.0f - (float)i / stacks };
	                vertices[curV].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	                vertices[curV].textureSlot = 0.0f;
	                vertices[curV].tangent = { 0.0f, 0.0f, 0.0f, 0.0f }; // 留给 MikkTSpace 填充
	            }
	        }

	        // --- 2. 生成球体网格索引 (严格遵循与你的 Cube 一致的 CCW 逆时针外翻绕序) ---
	        int localIdx = 0;
	        for (int i = 0; i < stacks; ++i)
	        {
	            for (int j = 0; j < sectors; ++j)
	            {
	                // 获取当前网格四边形的 4 个顶点索引
	                uint32_t k1 = vOffset + i * (sectors + 1) + j;
	                uint32_t k2 = k1 + 1;
	                uint32_t k3 = k1 + (sectors + 1);
	                uint32_t k4 = k3 + 1;

	                // 三角形 1
	                indices[iOffset + localIdx + 0] = k1;
	                indices[iOffset + localIdx + 1] = k3;
	                indices[iOffset + localIdx + 2] = k2;

	                // 三角形 2
	                indices[iOffset + localIdx + 3] = k2;
	                indices[iOffset + localIdx + 4] = k3;
	                indices[iOffset + localIdx + 5] = k4;

	                localIdx += 6;
	            }
	        }
	    }

	    std::vector<Vertex> verticesVec(vertices, vertices + SIZE * vPerSphere);
	    std::vector<uint32_t> indicesVec(indices, indices + SIZE * iPerSphere);
	    
	    CalcMikkTSpaceTangents(verticesVec, indicesVec, 3);
	    
	    std::memcpy(vertices, verticesVec.data(), verticesVec.size() * sizeof(Vertex));
	    std::memcpy(indices, indicesVec.data(), indicesVec.size() * sizeof(uint32_t));
	}
	
	void PBRTexturePhaser(std::string& albedo, std::string& metallic, std::string& roughness, std::string& ao, std::string& normal, const std::string& assetPath, const std::string& assetName, const std::string& normalFormat)
	{
		albedo = assetPath + assetName + "_albedo.png";
		metallic = assetPath + assetName + "_metallic.png";
		roughness = assetPath + assetName + "_roughness.png";
		ao = assetPath + assetName + "_ao.png";
		normal = assetPath + assetName + "_normal" + "-" + normalFormat + ".png";
	}
	
}