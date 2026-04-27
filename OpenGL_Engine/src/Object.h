#pragma once
#include <glad/glad.h>
#include <vector>

#include "VertexAttribArray.h"
#include "VertexArrayBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
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

		int m_VertexCount;
		int m_IndexCount;

		bool m_Cleared = false;
	public:
		Material* m_Material = nullptr;
		
		Object(const Vertex* vertices, const int* indices, int vertexCount, int indexCount);
		~Object();

		void OnDraw() const;
		 
		void Destroy();

		inline const VertexArrayBuffer& GetVAO() const { return m_VAO; }
		inline const VertexBuffer& GetVBO() const { return m_VBO; }
		inline const IndexBuffer& GetIBO() const { return m_IBO; }
		inline int GetIndexCount() const { return m_IndexCount; }
		inline int GetVertexCount() const { return m_VertexCount; }
		
		inline void BindMaterial(Material* material) { m_Material = material; }
		inline void UnBindMaterial() { m_Material = nullptr; }
	};
}

