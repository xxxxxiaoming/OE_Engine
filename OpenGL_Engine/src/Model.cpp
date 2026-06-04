#include "Model.h"


#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <mikktspace.h>

#include <cstdlib>

#include "Material.h"
#include "Type.h"

// 定义一个临时结构体，把你的网格数据传进去
struct MikkMeshData
{
	Engine::Vertex* vertices;
	uint32_t*       indices;
	size_t          indexCount;
	uint8_t         indicesOfOneFace;
};

void CalcMikkTSpaceTangents(Engine::Vertex* vertices, uint32_t* indices, uint8_t indicesPerFace, size_t indexCount)
{

    MikkMeshData meshData{ vertices, indices, indexCount, indicesPerFace };

    SMikkTSpaceInterface iface{};

    iface.m_getNumFaces = [](const SMikkTSpaceContext* ctx) -> int {
        auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
        return (int)data->indexCount / data->indicesOfOneFace;
    };

    iface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* ctx, const int) -> int {
    	auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
        return data->indicesOfOneFace;
    };

    iface.m_getPosition = [](const SMikkTSpaceContext* ctx, float out[], const int iFace, const int iVert) {
        auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
        const auto& pos = data->vertices[data->indices[iFace * data->indicesOfOneFace + iVert]].pos;
        out[0] = pos.x; out[1] = pos.y; out[2] = pos.z;
    };

    iface.m_getNormal = [](const SMikkTSpaceContext* ctx, float out[], const int iFace, const int iVert) {
        auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
        const auto& n = data->vertices[data->indices[iFace * data->indicesOfOneFace + iVert]].normal;
        out[0] = n.x; out[1] = n.y; out[2] = n.z;
    };

    iface.m_getTexCoord = [](const SMikkTSpaceContext* ctx, float out[], const int iFace, const int iVert) {
        auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
        const auto& uv = data->vertices[data->indices[iFace * data->indicesOfOneFace + iVert]].texCoord;
        out[0] = uv.x; out[1] = uv.y;
    };

    iface.m_setTSpaceBasic = [](const SMikkTSpaceContext* ctx, const float tangent[], const float fSign, const int iFace, const int iVert) {
        auto* data = static_cast<MikkMeshData*>(ctx->m_pUserData);
        auto& vert = data->vertices[data->indices[iFace * data->indicesOfOneFace + iVert]];
        vert.tangent   = Engine::vec3{tangent[0], tangent[1], tangent[2]};
        vert.bitangent = Engine::vec3{fSign, 0.0f, 0.0f};
    };

    SMikkTSpaceContext ctx{};
    ctx.m_pInterface = &iface;
    ctx.m_pUserData  = &meshData;

    genTangSpaceDefault(&ctx);
}

static Engine::BlendMode ProcessMeshBlendMode(const aiMaterial* material)
{
	aiString aiModeString;
	if (material->Get("$mat.gltf.alphaMode", 0, 0, aiModeString) == AI_SUCCESS)
	{
		if (std::strcmp(aiModeString.C_Str(), "BLEND") == 0)
			return Engine::BlendMode::Transparent;
		else if (std::strcmp(aiModeString.C_Str(), "MASK") == 0)
			return Engine::BlendMode::Masked;
		else
			return Engine::BlendMode::Opaque;
	}
	
	float opacity = 1.0f;
	if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
	{
		if (opacity < 1.0f)
			return Engine::BlendMode::Transparent;
		else
			return Engine::BlendMode::Opaque;
	}
	
	int blendMode = 0;
	if (material->Get(AI_MATKEY_BLEND_FUNC, blendMode) == AI_SUCCESS)
	{
		if (blendMode == aiBlendMode_Additive)
			return Engine::BlendMode::Transparent;
		else
			return Engine::BlendMode::Opaque;
	}
	
	return Engine::BlendMode::Opaque;
}

Engine::Part::Part(const aiNode* node, const aiScene* scene, uint8_t indicesOfOneFace, std::string& assetDirectory, std::string& format) :
	indicesPerFace(indicesOfOneFace)
{
	objects.reserve(node->mNumMeshes);
	for (uint32_t index = 0; index < node->mNumMeshes; index++)
	{
		uint32_t meshIndex = node->mMeshes[index];
		aiMesh* mesh = scene->mMeshes[meshIndex];
		ProcessMesh(mesh, scene, assetDirectory, format);
	}

	name = node->mName.C_Str();
}

void Engine::Part::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string& assetDirectory, std::string& format)
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
		
		if (indicesPerFace < 3 && mesh->HasTangentsAndBitangents())
		// if (mesh->HasTangentsAndBitangents())
		{
			aiVector3D aiTangent = mesh->mTangents[count];
			aiVector3D aiBitangent = mesh->mBitangents[count];
			vertices[count].bitangent = vec3{ aiBitangent.x, aiBitangent.y, aiBitangent.z };
			vertices[count].tangent = vec3{ aiTangent.x, aiTangent.y, aiTangent.z };
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
	
	// if (false)
	if (indicesPerFace >= 3)
		// Attention pls! MikkTSpace 只支持计算三角形/四边形片元的切线空间
		CalcMikkTSpaceTangents(vertices, indices, indicesPerFace, mesh->mNumFaces * indicesPerFace);
	
	objects.emplace_back(vertices, indices, mesh->mNumVertices, mesh->mNumFaces * indicesPerFace, assetDirectory);

	/* Load textures */
	uint32_t materialIndex = mesh->mMaterialIndex;
	Object& object = objects.back();
		
	aiMaterial* material = scene->mMaterials[materialIndex];
	uint32_t ambientNum = material->GetTextureCount(aiTextureType_AMBIENT);
	uint32_t diffuseNum = material->GetTextureCount(aiTextureType_DIFFUSE);
	uint32_t specularNum = material->GetTextureCount(aiTextureType_SPECULAR);
	uint32_t reflctNum = material->GetTextureCount(aiTextureType_REFLECTION);
	
	ambientNum = ambientNum > MAX_TEXTURES ? MAX_TEXTURES : ambientNum;
	diffuseNum = diffuseNum > MAX_TEXTURES ? MAX_TEXTURES : diffuseNum;
	specularNum = specularNum > MAX_TEXTURES ? MAX_TEXTURES : specularNum;

	object.m_TexturesAmbient.reserve(ambientNum);
	object.m_TexturesDiffuse.reserve(diffuseNum);
	object.m_TextureSpecular.reserve(specularNum);
		
	for (uint32_t count = 0; count < ambientNum; count++)
	{
		aiString path;
		std::string fullPath = assetDirectory;
		material->GetTexture(aiTextureType_AMBIENT, count, &path);
		object.m_TexturesAmbient.emplace_back(fullPath.append(path.C_Str()));
			
		/* 若没有漫反射贴图，则使用环境光贴图作为漫反射贴图 */
		if (diffuseNum == 0)
			object.m_TexturesDiffuse.emplace_back(fullPath);
	}

		
	for (uint32_t count = 0; count < diffuseNum; count++)
	{
		aiString path;
		std::string fullPath = assetDirectory;
		material->GetTexture(aiTextureType_DIFFUSE, count, &path);
		object.m_TexturesDiffuse.emplace_back(fullPath.append(path.C_Str()));
			
		/* 若没有环境光反射贴图，则使用漫反射贴图作为环境光贴图 */
		if (ambientNum == 0)object.m_TexturesAmbient.emplace_back(fullPath);
	}

	if(specularNum == 0)
	{
		if (reflctNum > 0)
		{
			object.m_TextureSpecular.reserve(reflctNum);
			for (uint32_t count = 0; count < reflctNum; count++)
			{
				aiString path;
				std::string fullPath = assetDirectory;
				material->GetTexture(aiTextureType_REFLECTION, count, &path);
				object.m_TextureSpecular.emplace_back(fullPath.append(path.C_Str()));
			}
		}
		else
		{
			/* 没有高光贴图的mesh绑定一张纯黑的默认贴图 */
			object.m_TextureSpecular.reserve(1);
			object.m_TextureSpecular.emplace_back("res/texture/default_specular.jpg");	
		}
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
	
	/* 法线贴图 */
	if (format == "obj")
	{
		uint32_t normalNum = material->GetTextureCount(aiTextureType_HEIGHT);
		normalNum = normalNum > MAX_TEXTURES ? MAX_TEXTURES : normalNum;
		
		if (normalNum > 0)
		{
			object.m_TextureNormal.reserve(normalNum);
			for (uint32_t count = 0; count < normalNum; count++)
			{
				aiString path;
				std::string fullPath = assetDirectory;
				material->GetTexture(aiTextureType_HEIGHT, count, &path);
				object.m_TextureNormal.emplace_back(fullPath.append(path.C_Str()));
			}
		}
		else
		{
			object.m_TextureNormal.reserve(1);
			object.m_TextureNormal.emplace_back(0xFFFF8080);
		}
	}
	else
	{
		// TODO : Deal with other formats
	}
	
	object.SetBlendMode(ProcessMeshBlendMode(material));

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

Engine::Model::Model(const std::string& path, bool bFlipUV, const Transform& transform) :
	m_ModelPath(path),
	m_Transform(GenerateModelMatrix(transform)),
	m_NormalMatrix(glm::transpose(glm::inverse(glm::mat3(m_Transform))))
{
	printf("Loading ...\n");
	Assimp::Importer importer;
	uint32_t importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
	
	if (bFlipUV)
		importFlags |= aiProcess_FlipUVs;
	
	const aiScene* scene = importer.ReadFile(path, importFlags);
	
	uint32_t nodeNum = 1;
	GetChildrenNum(scene->mRootNode, nodeNum);	
	m_Parts.reserve(nodeNum);

	std::string assetDirectory = path.substr(0, path.find_last_of('/') + 1);
	std::string format = path.substr(path.find_last_of('.') + 1);
	ProcessNode(scene->mRootNode, scene, assetDirectory, format);

	printf("Loading completed\n");
}

void Engine::Model::ProcessNode(const aiNode* node, const aiScene* scene, std::string& assetDirectory, std::string& format)
{
	printf("Process Node: %s\n", node->mName.C_Str());
	m_Parts.emplace_back(node, scene, 3, assetDirectory, format);
	for (uint32_t count = 0; count < node->mNumChildren; count++)
		ProcessNode(node->mChildren[count], scene, assetDirectory, format);
}

void Engine::Model::GetChildrenNum(aiNode* node, uint32_t& count)
{
	if (node == nullptr || node->mNumChildren == 0)
		return;
	
	count += node->mNumChildren;
	
	for (uint32_t index = 0; index < node->mNumChildren; index++)
		GetChildrenNum(node->mChildren[index], count);
}

size_t Engine::Model::GetPartsCount() const
{
	return m_Parts.size();
}

size_t Engine::Model::GetObjectsCount() const
{
	size_t count = 0;
	for (auto& part : m_Parts)
		count += part.objects.size();
	
	return count;
}

void Engine::Model::BindShader(Shader* shader)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.shader = shader;
}

void Engine::Model::BindAmbientSlot(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindAmbientSlots(slots, slotsNum);
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

void Engine::Model::BindNormalSlot(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindNormalSlots(slots, slotsNum);
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

void Engine::Model::DrawInstanced(const Renderer& renderer, uint32_t amount)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
		{
			object.OnDraw();
			renderer.DrawElementsInstanced(object.GetIndexCount(), nullptr, amount);
		}
}

void Engine::Model::BindInstancedVertexAttrib(int index, int size, int type, int stride, int offset, uint32_t divisor)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
		{
			VertexArrayBuffer& VAO = object.GetVAO();
			VAO.Bind();
			VertexAttribArray::Enable(index);
			VertexAttribArray::SetPointer(index, size, type, GL_FALSE, stride, offset);
			VertexAttribArray::SetAttributeDivisor(index, divisor);
			VAO.Unbind();
		}
}

void Engine::Model::EnableLight()
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.EnableLight();
}

void Engine::Model::DisableLight()
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.DisableLight();
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