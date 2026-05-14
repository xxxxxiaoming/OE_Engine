#include "Model.h"


#include <assimp/postprocess.h>
#include <assimp/vector3.h>

#include <cstdlib>

#include "Material.h"
#include "Type.h"

Engine::Part::Part(const aiNode* node, const aiScene* scene, uint8_t indicesOfOneFace, std::string& assetDirectory) :
	indicesPerFace(indicesOfOneFace)
{
	objects.reserve(node->mNumMeshes);
	for (uint32_t index = 0; index < node->mNumMeshes; index++)
	{
		uint32_t meshIndex = node->mMeshes[index];
		aiMesh* mesh = scene->mMeshes[meshIndex];
		ProcessMesh(mesh, scene, assetDirectory);
	}

	name = node->mName.C_Str();
}

void Engine::Part::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string& assetDirectory)
{
	if (!mesh->HasPositions())
		return;

	Vertex* vertices = static_cast<Vertex*>(std::malloc(sizeof(Vertex) * mesh->mNumVertices));
	
	for (uint32_t count = 0; count < mesh->mNumVertices; count++)
	{
		aiVector3D& aiVertex = mesh->mVertices[count];
		vertices[count].pos = vec3{ aiVertex.x, aiVertex.y, aiVertex.z };

		if (mesh->HasNormals())
		{
			aiVector3D& aiNormal = mesh->mNormals[count];
			vertices[count].normal = vec3{ aiNormal.x, aiNormal.y, aiNormal.z };
		}

		if (mesh->HasTextureCoords(0))
		{
			aiVector3D aiTextureCoord = mesh->mTextureCoords[0][count];
			vertices[count].texCoord = vec2{ aiTextureCoord.x, aiTextureCoord.y };
		}

		if (mesh->HasVertexColors(0))
		{
			aiColor4D aiVertexColor = mesh->mColors[0][count];
			vertices[count].color = vec4{ aiVertexColor.r, aiVertexColor.g, aiVertexColor.b, aiVertexColor.a };
		}

		vertices[count].textureSlot = 0.0f;
	}

	uint32_t* indices = static_cast<uint32_t*>(std::malloc(sizeof(uint32_t) * mesh->mNumFaces * indicesPerFace));
	assert(indices != nullptr);
	
	for (uint32_t count = 0; count < mesh->mNumFaces; count++)
	{
		aiFace face = mesh->mFaces[count];
		for(int index = 0; index < indicesPerFace; index ++)
			indices[count * indicesPerFace + index] = face.mIndices[index];
	}
	
	objects.emplace_back(vertices, indices, mesh->mNumVertices, mesh->mNumFaces * indicesPerFace, assetDirectory);

	/* Load textures */
	uint32_t materialIndex = mesh->mMaterialIndex;
	if (materialIndex != 0)
	{
		Object& object = objects.back();
		
		aiMaterial* material = scene->mMaterials[materialIndex];
		uint32_t diffuseNum = material->GetTextureCount(aiTextureType_DIFFUSE);
		uint32_t specularNum = material->GetTextureCount(aiTextureType_SPECULAR);
		uint32_t normalNum = material->GetTextureCount(aiTextureType_NORMALS);
		
		diffuseNum = diffuseNum > MAX_TEXTURES ? MAX_TEXTURES : diffuseNum;
		specularNum = specularNum > MAX_TEXTURES ? MAX_TEXTURES : specularNum;

		object.m_TexturesDiffuse.reserve(diffuseNum);
		object.m_TextureSpecular.reserve(specularNum);

		for (uint32_t count = 0; count < diffuseNum; count++)
		{
			aiString path;
			std::string fullPath = assetDirectory;
			material->GetTexture(aiTextureType_DIFFUSE, count, &path);
			object.m_TexturesDiffuse.emplace_back(fullPath.append(path.C_Str()));
		}

		if(specularNum == 0)
		{
			/* 没有高光贴图的mesh绑定一张纯黑的默认贴图 */
			object.m_TextureSpecular.reserve(1);
			object.m_TextureSpecular.emplace_back("res/texture/default_specular.jpg");
		}
		else
		{
			for (uint32_t count = 0; count < specularNum; count++)
			{
				aiString path;
				std::string fullPath = assetDirectory;
				material->GetTexture(aiTextureType_SPECULAR, count, &path);
				object.m_TextureSpecular.emplace_back(fullPath.append(path.C_Str()));
			}
		}
	}

	std::free(vertices);
	std::free(indices);
}

void Engine::Part::DestroyPart()
{
	if (!m_Destroyed)
	{
		for (auto& object : objects)
		{
			object.Destroy();
		}

		m_Destroyed = true;
	}
}

Engine::Part::~Part()
{
	if (!m_Destroyed)
	{
		DestroyPart();
		m_Destroyed = true;
	}
}

Engine::Model::Model(const std::string& path) :
	m_ModelPath(path)
{
	printf("Loading ...\n");
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	
	uint32_t nodeNum = 1;
	GetChildrenNum(scene->mRootNode, nodeNum);	
	m_Parts.reserve(nodeNum);

	std::string assetDirectory = path.substr(0, path.find_last_of('/') + 1);
	ProcessNode(scene->mRootNode, scene, assetDirectory);

	printf("Loading completed\n");
}

void Engine::Model::ProcessNode(const aiNode* node, const aiScene* scene, std::string& assetDirectory)
{
	printf("Process Node: %s\n", node->mName.C_Str());
	m_Parts.emplace_back(node, scene, 3, assetDirectory);
	for (uint32_t count = 0; count < node->mNumChildren; count++)
		ProcessNode(node->mChildren[count], scene, assetDirectory);
}

void Engine::Model::GetChildrenNum(aiNode* node, uint32_t& count)
{
	if (node == nullptr || node->mNumChildren == 0)
		return;
	
	count += node->mNumChildren;
	
	for (uint32_t index = 0; index < node->mNumChildren; index++)
		GetChildrenNum(node->mChildren[index], count);
}

void Engine::Model::BindShader(Shader* shader)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.shader = shader;
}

void Engine::Model::BindDiffuseSlot(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindDiffuseSlots(slots, slotsNum);
}

void Engine::Model::BindSpecularSlot(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindSpecularSlots(slots, slotsNum);
}

void Engine::Model::Draw(const Renderer& renderer)
{ 
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
		{
			object.OnDraw();
			renderer.DrawElements(object.GetIndexCount(), nullptr);
		}
}

Engine::Model::~Model()
{
	if (!m_Destroyed)
		Destroy();
}

void Engine::Model::Destroy()
{
	if (!m_Destroyed)
	{
		for (auto& part : m_Parts)
			part.DestroyPart();
		m_Destroyed = true;
	}
}