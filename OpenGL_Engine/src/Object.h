#pragma once
#include <glad/glad.h>
#include <vector>
#include <assimp/material.h>

#include "VertexAttribArray.h"
#include "VertexArrayBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "Texture.h"
#include "Material.h"
#include "Type.h"

namespace Engine
{
	class Object
	{
	private:
		VertexArrayBuffer m_VAO;
		VertexBuffer m_VBO;
		IndexBuffer m_IBO;

		uint32_t m_VertexCount;
		uint32_t m_IndexCount;

		bool m_Cleared = false;
		bool m_EnableLight = false;
	public:
		std::string m_AssetDirectory;
		
		Material m_Material;
		std::vector<Texture> m_TexturesDiffuse;
		std::vector<Texture> m_TextureSpecular;
		
		Object(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory);
		~Object();

		void OnDraw();
		 
		void Destroy();

		inline void EnableLight() { m_EnableLight = true; }
		inline void DisableLight() { m_EnableLight = false; }

		inline VertexArrayBuffer& GetVAO() { return m_VAO; }
		inline VertexBuffer& GetVBO() { return m_VBO; }
		inline IndexBuffer& GetIBO()  { return m_IBO; }
		inline uint32_t GetIndexCount() const { return m_IndexCount; }
		inline uint32_t GetVertexCount() const { return m_VertexCount; }
	};
}

