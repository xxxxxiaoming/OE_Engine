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
#include "EngineConfig.h"

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
		
		BlendMode m_BlendMode = BlendMode::Opaque;
		
		glm::vec3 m_Position;
		glm::mat4 m_Transform;
		glm::mat3 m_NormalMatrix;
	public:
		Material m_Material;
		
		std::string m_AssetDirectory;
		std::vector<Texture> m_TextureNormal;
		std::vector<Texture> m_TextureEmissive;
		
#ifdef PBR_PIPELINE
		// 各种纹理图
		std::vector<Texture> m_TextureAlbedo;
		std::vector<Texture> m_TextureMetallic;
		std::vector<Texture> m_TextureRoughness;
		std::vector<Texture> m_TextureAO;
		std::vector<Texture> m_TextureTransmission;
		bool m_UseMRA = false;

#elif defined(BLING_PHONG_PIPELINE)
		std::vector<Texture> m_TexturesAmbient;
		std::vector<Texture> m_TexturesDiffuse;
		std::vector<Texture> m_TextureSpecular;
#endif
		

		
		Object();
		Object(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory, const Transform& transform = Transform{}, BlendMode blendMode = BlendMode::Opaque);
		~Object();

		void OnDraw();
		 
		void Destroy();

		void EnableLight() { m_EnableLight = true; }
		void DisableLight() { m_EnableLight = false; }
		
		Object& operator()(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory, const Transform& transform = Transform{});

		VertexArrayBuffer& GetVAO() { return m_VAO; }
		VertexBuffer& GetVBO() { return m_VBO; }
		IndexBuffer& GetIBO()  { return m_IBO; }
		uint32_t GetIndexCount() const { return m_IndexCount; }
		uint32_t GetVertexCount() const { return m_VertexCount; }
		glm::mat4 GetTransform() const { return m_Transform; }
		glm::vec3 GetPosition() const { return m_Position; }
		glm::mat3 GetNormalMatrix() const { return m_NormalMatrix; }
		BlendMode GetBlendMode() const { return m_BlendMode; }
		
		void SetTransform(const Transform& transform)
		{
			m_Position = transform.translation;
			m_Transform = GenerateModelMatrix(transform);
			m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Transform)));
		}
		
		void SetTransform(const glm::mat4& transform)
		{
			m_Position = glm::vec3(transform[3]);
			m_Transform = transform;
			m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Transform)));
		}
		
		void SetBlendMode(BlendMode blendMode)
		{
			m_BlendMode = blendMode;
		}
	};
}

