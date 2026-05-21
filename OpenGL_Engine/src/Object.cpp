#include <string>
#include <assimp/types.h>
#include "Object.h"

Engine::Object::Object(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory, const Transform& transform) :  
	m_VertexCount(vertexCount), 
	m_IndexCount(indexCount), 
	m_AssetDirectory(assetDirectory), 
	m_Transform(GenerateModelMatrix(transform)), 
	m_NormalMatrix(glm::transpose(glm::inverse(glm::mat3(m_Transform))))
{
	m_VAO.Bind();
	
	m_VBO.Bind();
	m_VBO.SetBufferData(sizeof(Vertex) * vertexCount, vertices, GL_STATIC_DRAW);
	
	m_IBO.Bind();
	m_IBO.SetBufferData(sizeof(uint32_t) * indexCount, indices, GL_STATIC_DRAW);

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

void Engine::Object::OnDraw()
{
	m_VAO.Bind();
	
	/* Model.cpp load texture 的时候，已经限制了load texture的数量为MAX_TEXTURES，跟一个Material对象能够使用的纹理数量一致 */
	for (int index = 0; index < m_TexturesAmbient.size(); index++)
		m_TexturesAmbient[index].Bind(m_Material.ambient[index]);
	
	for (int index = 0; index < m_TexturesDiffuse.size(); index++)
		m_TexturesDiffuse[index].Bind(m_Material.diffuse[index]);

	for (int index = 0; index < m_TextureSpecular.size(); index++)
		m_TextureSpecular[index].Bind(m_Material.specular[index]);
	
	/* Maybe different object will have different shader in the future. So I decide to call UseMaterial here. */
	/* 开启 phong light的情况下，在 PhongLight::TurnOn() 中 use shader */
	if (!m_EnableLight)
	{
		m_Material.UseMaterial();
		return;
	}
	
	/* 目前先约定shader中，material结构体都是按照由下面的数据构成的。 */
	/* 目前先固定使用1张diffuse跟1张specular，之后再扩展了 */
	m_Material.shader->SetUniform1i("u_Material.ambient", m_Material.ambient[0]);
	m_Material.shader->SetUniform1i("u_Material.diffuse", m_Material.diffuse[0]);
	m_Material.shader->SetUniform1i("u_Material.specular", m_Material.specular[0]);
	m_Material.shader->SetUniform1i("u_Material.shininess", m_Material.shininess);
}

void Engine::Object::Destroy()
{
	if (m_Cleared)
		return;

	m_VAO.Delete();
	m_VBO.DeleteBuffer();
	m_IBO.DeleteBuffer();

	for (auto& texture : m_TexturesDiffuse)
		texture.Delete();

	for (auto& texture : m_TextureSpecular)
		texture.Delete();

	m_Cleared = true;
}
