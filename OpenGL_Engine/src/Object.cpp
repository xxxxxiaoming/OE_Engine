#include "Object.h"

Engine::Object::Object(const Vertex* vertices, const int* indices, int vertexCount, int indexCount) :  
	m_VertexCount(vertexCount), m_IndexCount(indexCount)
{
	m_VAO.Bind();
	
	m_VBO.Bind();
	m_VBO.SetBufferData(sizeof(Vertex) * vertexCount, vertices, GL_STATIC_DRAW);
	
	m_IBO.Bind();
	m_IBO.SetBufferData(sizeof(int) * indexCount, indices, GL_STATIC_DRAW);

	VertexAttribArray::Enable(0);
	VertexAttribArray::Enable(1);
	VertexAttribArray::Enable(2);
	VertexAttribArray::Enable(3);
	VertexAttribArray::Enable(4);
	
	VertexAttribArray::SetPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, pos));
	VertexAttribArray::SetPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
	VertexAttribArray::SetPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, textureSlot));
	VertexAttribArray::SetPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
	VertexAttribArray::SetPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));

	m_VAO.Unbind();
}

Engine::Object::~Object()
{
	if (!m_Cleared)
		Destroy();
}

void Engine::Object::OnDraw() const
{
	m_VAO.Bind();

	if(m_Material != nullptr)
	{
		m_Material->UseMaterial();
	}
}

void Engine::Object::Destroy()
{
	if (m_Cleared)
		return;

	m_VAO.Delete();
	m_VBO.DeleteBuffer();
	m_IBO.DeleteBuffer();

	m_Cleared = true;
}
