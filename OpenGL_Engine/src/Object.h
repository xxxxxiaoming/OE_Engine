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
		
		glm::mat4 m_Transform;
		glm::mat3 m_NormalMatrix;
	public:
		std::string m_AssetDirectory;
		
		Material m_Material;
		std::vector<Texture> m_TexturesAmbient;
		std::vector<Texture> m_TexturesDiffuse;
		std::vector<Texture> m_TextureSpecular;
		std::vector<Texture> m_TextureNormal;
		
		Object(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory, const Transform& transform = Transform{});
		~Object();

		void OnDraw();
		 
		void Destroy();

		void EnableLight() { m_EnableLight = true; }
		void DisableLight() { m_EnableLight = false; }

		VertexArrayBuffer& GetVAO() { return m_VAO; }
		VertexBuffer& GetVBO() { return m_VBO; }
		IndexBuffer& GetIBO()  { return m_IBO; }
		uint32_t GetIndexCount() const { return m_IndexCount; }
		uint32_t GetVertexCount() const { return m_VertexCount; }
		glm::mat4 GetTransform() const { return m_Transform; }
		glm::mat3 GetNormalMatrix() const { return m_NormalMatrix; }
		
		void SetTransform(const Transform& transform)
		{
			m_Transform = GenerateModelMatrix(transform);
			m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Transform)));
		}
	};
}

