#include <string>
#include <assimp/types.h>
#include "Object.h"

Engine::Object::Object() :
	m_VertexCount(0), m_IndexCount(0), m_Transform(glm::mat4(1.0f)), m_NormalMatrix(glm::mat4(1.0f))
{
	
}

Engine::Object::Object(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory, const Transform& transform, BlendMode blendMode) :  
	m_VertexCount(vertexCount), 
	m_IndexCount(indexCount), 
	m_AssetDirectory(assetDirectory), 
	m_BlendMode(blendMode),
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
	VertexAttribArray::Enable(5);
	
	VertexAttribArray::SetPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, pos));
	VertexAttribArray::SetPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
	VertexAttribArray::SetPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, textureSlot));
	VertexAttribArray::SetPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
	VertexAttribArray::SetPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));
	VertexAttribArray::SetPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, tangent));

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
#ifdef PBR_PIPELINE
	// 将纹理绑定到对应的槽位上
	for (int index = 0; index < m_TextureAlbedo.size(); index++)
		m_TextureAlbedo[index].Bind(m_Material.albedo[index]);
	
	for (int index = 0; index < m_TextureMetallic.size(); index++)
		m_TextureMetallic[index].Bind(m_Material.metallic[index]);

	for (int index = 0; index < m_TextureRoughness.size(); index++)
		m_TextureRoughness[index].Bind(m_Material.roughness[index]);
	
	for (int index = 0; index < m_TextureAO.size(); index++)
		m_TextureAO[index].Bind(m_Material.ao[index]);
	
	if (GetBlendMode() == BlendMode::Transparent or GetBlendMode() == BlendMode::TransparentMasked)
	{
		for (int index = 0; index < m_TextureTransmission.size(); index++)
			m_TextureTransmission[index].Bind(m_Material.transmission[index]);
	}
	
#elif defined(BLING_PHONG_PIPELINE)
	for (int index = 0; index < m_TexturesAmbient.size(); index++)
		m_TexturesAmbient[index].Bind(m_Material.ambient[index]);
	
	for (int index = 0; index < m_TexturesDiffuse.size(); index++)
		m_TexturesDiffuse[index].Bind(m_Material.diffuse[index]);

	for (int index = 0; index < m_TextureSpecular.size(); index++)
		m_TextureSpecular[index].Bind(m_Material.specular[index]);
#endif
	
	for (int index = 0; index < m_TextureNormal.size(); index++)
		m_TextureNormal[index].Bind(m_Material.normal[index]);
	
	for (int index = 0; index < m_TextureEmissive.size(); index++)
		m_TextureEmissive[index].Bind(m_Material.emissive[index]);
	
	/* Maybe different object will have different shader in the future. So I decide to call UseMaterial here. */
	/* 开启 phong light的情况下，在 PhongLight::TurnOn() 中 use shader */
	if (!m_EnableLight)
	{
		m_Material.UseMaterial();
		return;
	}
	
	/* 目前先约定shader中，material结构体都是按照由下面的数据构成的。 */
#ifdef PBR_PIPELINE
	// 告诉 shader 各种纹理所在的槽位
	m_Material.shader->SetUniform1i("u_Material.albedo", m_Material.albedo[0]);
	m_Material.shader->SetUniform1i("u_Material.metallic", m_Material.metallic[0]);
	m_Material.shader->SetUniform1i("u_Material.roughness", m_Material.roughness[0]);
	m_Material.shader->SetUniform1i("u_Material.AO", m_Material.ao[0]);
	
	m_Material.shader->SetUniform1i("u_UseMRA", m_UseMRA);
	m_Material.shader->SetUniform1f("u_Material.metallicFactor", m_Material.metallicFactor);
	m_Material.shader->SetUniform1f("u_Material.roughnessFactor", m_Material.roughnessFactor);
	m_Material.shader->SetUniform4f("u_Material.albedoFactor", m_Material.albedoFactor.r, m_Material.albedoFactor.g, m_Material.albedoFactor.b, m_Material.albedoFactor.a);
	
	if (GetBlendMode() == BlendMode::Transparent)
	{
		m_Material.shader->SetUniform1i("u_Material.transmission", m_Material.transmission[0]);
		m_Material.shader->SetUniform1f("u_Material.transmissionFactor", m_Material.transmissionFactor);
		m_Material.shader->SetUniform1f("u_Material.ior", m_Material.ior);
	}
	else if (GetBlendMode() == BlendMode::Masked)
	{
		m_Material.shader->SetUniform1f("u_Material.cutOff", m_Material.cutOff);
		m_Material.shader->SetUniform1i("u_Material.IsMask", true);
	}
	else
	{
		m_Material.shader->SetUniform1i("u_Material.IsMask", false);
	}
	
	
#elif defined(BLING_PHONG_PIPELINE)
	
	m_Material.shader->SetUniform1i("u_Material.ambient", m_Material.ambient[0]);
	m_Material.shader->SetUniform1i("u_Material.diffuse", m_Material.diffuse[0]);
	m_Material.shader->SetUniform1i("u_Material.specular", m_Material.specular[0]);
	m_Material.shader->SetUniform1i("u_Material.shininess", m_Material.shininess);
	
#endif
	
	m_Material.shader->SetUniform1i("u_Material.normal", m_Material.normal[0]);
	m_Material.shader->SetUniform1i("u_Material.emissive", m_Material.emissive[0]);
	
}

Engine::Object& Engine::Object::operator()(const Vertex* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t indexCount, std::string& assetDirectory, const Transform& transform)
{
	m_VertexCount = vertexCount;
	m_IndexCount = indexCount;
	m_AssetDirectory = assetDirectory;
	m_Transform = GenerateModelMatrix(transform);
	m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Transform)));
	
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
	VertexAttribArray::Enable(5);
	
	VertexAttribArray::SetPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, pos));
	VertexAttribArray::SetPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
	VertexAttribArray::SetPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, textureSlot));
	VertexAttribArray::SetPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
	VertexAttribArray::SetPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));
	VertexAttribArray::SetPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, tangent));

	m_VAO.Unbind();
	
	return *this;
}

void Engine::Object::Destroy()
{
	if (m_Cleared)
		return;

	m_VAO.Delete();
	m_VBO.DeleteBuffer();
	m_IBO.DeleteBuffer();

#ifdef PBR_PIPELINE
	for (auto& texture : m_TextureAlbedo)
		texture.Delete();

	for (auto& texture : m_TextureMetallic)
		texture.Delete();
	
	for (auto& texture : m_TextureRoughness)
		texture.Delete();

	for (auto& texture : m_TextureAO)
		texture.Delete();
	
#elif defined(BLING_PHONG_PIPELINE)
	for (auto& texture : m_TexturesDiffuse)
		texture.Delete();

	for (auto& texture : m_TextureSpecular)
		texture.Delete();
	
	for (auto& texture : m_TexturesAmbient)
		texture.Delete();
	
#endif

	for (auto& texture : m_TextureNormal)
		texture.Delete();
	
	m_Cleared = true;
}
