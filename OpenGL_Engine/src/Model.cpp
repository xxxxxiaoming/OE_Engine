#include "Model.h"


#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <assimp/GltfMaterial.h>

#include <cstdlib>

#include "Material.h"
#include "Type.h"
#include "EngineConfig.h"
#include "Helper.h"

static uint32_t ColorToRGBA(const aiColor4D& color)
{
	uint8_t r = static_cast<uint8_t>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
	uint8_t g = static_cast<uint8_t>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
	uint8_t b = static_cast<uint8_t>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
	uint8_t a = static_cast<uint8_t>(std::clamp(color.a, 0.0f, 1.0f) * 255.0f);

	// 对应小端序内存布局：低字节到高字节分别为 R, G, B, A
	return (a << 24) | (b << 16) | (g << 8) | r;
}

static Engine::BlendMode ProcessMeshBlendMode(const aiMaterial* material)
{
	// 第一优先级：检查是否有 transmission（不管 alphaMode 是什么）
	float transmissionFactor = 0.0f;
	material->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmissionFactor);

	// 第二优先级：检查 alphaMode
	aiString aiModeString;
	material->Get(AI_MATKEY_GLTF_ALPHAMODE, aiModeString);
	
	float opacity = 1.0f;
	material->Get(AI_MATKEY_OPACITY, opacity);
	
	bool hasTransmission = transmissionFactor > 0.01f;
	bool hasMask = std::strcmp(aiModeString.C_Str(), "MASK") == 0;
	bool hasBlend = std::strcmp(aiModeString.C_Str(), "BLEND") == 0;
	bool isTransparent = opacity < 0.99f;
	
	if (hasTransmission && hasMask)
		return Engine::BlendMode::TransparentMasked;
	else if (hasTransmission || hasBlend || isTransparent)
		return Engine::BlendMode::Transparent;
	else if (hasMask)
		return Engine::BlendMode::Masked;
	else 
		return Engine::BlendMode::Opaque;
}

static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
	glm::mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

#ifdef BLING_PHONG_PIPELINE
static void LoadMeshTexturesBlingPhongPipeline(aiMaterial* material, Engine::Object& object, const std::string& format, const::std string& assetDirectory)
{
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
			object.m_TextureSpecular.emplace_back(0xFF000000);	
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
	if (format == "obj" || format == "gltf")
	{
		aiTextureType flag = aiTextureType_NORMALS;
		uint32_t normalNum = material->GetTextureCount(flag);
		normalNum = normalNum > MAX_TEXTURES ? MAX_TEXTURES : normalNum;
		
		if (normalNum == 0)
		{
			flag = aiTextureType_NORMALS;
			normalNum = material->GetTextureCount(flag);
		}
		
		if (normalNum > 0)
		{
			object.m_TextureNormal.reserve(normalNum);
			for (uint32_t count = 0; count < normalNum; count++)
			{
				aiString path;
				std::string fullPath = assetDirectory;
				material->GetTexture(flag, count, &path);
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
}
#endif

#ifdef PBR_PIPELINE
static void LoadMeshTexturePBRPipeline(aiMaterial* material, Engine::Object& object, const std::string& format, const std::string& assetDirectory)
{
	if (format == "gltf")
	{
		aiString albedo;
		aiString roughness;
		aiString metallic;
		aiString occlusion;
		aiString normal;
		aiString emissive;
		aiString transmission;
		
		float metallicFactor = 0.0f;
		float roughnessFactor =  1.0f;
		float transmissionFactor = 0.0f;
		aiColor4D basecolorFactor{1.0f, 1.0f, 1.0f, 1.0f};
		float ior = 1.5f;
		
		object.m_TextureAlbedo.reserve(4);
		object.m_TextureRoughness.reserve(4);
		object.m_TextureMetallic.reserve(4);
		object.m_TextureAO.reserve(4);
		object.m_TextureNormal.reserve(4);
		object.m_TextureEmissive.reserve(4);
		
		// Albedo
		material->Get(AI_MATKEY_BASE_COLOR, basecolorFactor);
		object.m_Material.albedoFactor = {basecolorFactor.r, basecolorFactor.g, basecolorFactor.b, basecolorFactor.a};
		if (material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &albedo) == AI_SUCCESS)
		{
			std::string fullPath = assetDirectory + albedo.C_Str();
			object.m_TextureAlbedo.emplace_back(fullPath);
		}
		else
		{
			object.m_TextureAlbedo.emplace_back(0xFFFFFFFF);
		}
		
		// Roughness
		material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
		object.m_Material.roughnessFactor = roughnessFactor;
		if (material->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughness) == AI_SUCCESS)
		{
			std::string fullPath = assetDirectory + roughness.C_Str();
			object.m_TextureRoughness.emplace_back(fullPath);
		}
		else
		{
			object.m_TextureRoughness.emplace_back(0xFFFFFFFF);
		}
		
		// Metallic
		material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
		object.m_Material.metallicFactor = metallicFactor;
		if (material->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallic) == AI_SUCCESS)
		{
			std::string fullPath = assetDirectory + metallic.C_Str();
			object.m_TextureMetallic.emplace_back(fullPath);
		}
		else
		{
			object.m_TextureMetallic.emplace_back(0xFFFFFFFF);
		}
		
		// Transmission
		if (object.GetBlendMode() == Engine::BlendMode::Transparent)
		{
			if (material->GetTexture(AI_MATKEY_TRANSMISSION_TEXTURE, &transmission) == AI_SUCCESS)
			{
				std::string fullPath = assetDirectory + transmission.C_Str();
				object.m_TextureTransmission.emplace_back(fullPath);
			}
			else
			{
				object.m_TextureTransmission.emplace_back(0xFFFFFFFF);
			}
			
			material->Get(AI_MATKEY_REFRACTI, ior);
			material->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmissionFactor);
			object.m_Material.transmissionFactor = transmissionFactor;
			object.m_Material.ior = ior;
		}
		
		if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusion) == AI_SUCCESS)
		{
			std::string fullPath = assetDirectory + occlusion.C_Str();
			object.m_TextureAO.emplace_back(fullPath);
		}
		else
		{
			object.m_TextureAO.emplace_back(0xFFFFFFFF);
		}
		
		if (material->GetTexture(aiTextureType_NORMALS, 0, &normal) == AI_SUCCESS)
		{
			std::string fullPath = assetDirectory + normal.C_Str();
			object.m_TextureNormal.emplace_back(fullPath);
		}
		else
		{
			object.m_TextureNormal.emplace_back(0xFFFF8080);
		}
		
		if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissive) == AI_SUCCESS)
		{
			std::string fullPath = assetDirectory + emissive.C_Str();
			object.m_TextureEmissive.emplace_back(fullPath);
		}
		else
		{
			object.m_TextureEmissive.emplace_back(0xFF000000);
		}
		
		// gltf格式模型，metallic/roughness/ao 三者共用一张贴图
		object.m_UseMRA = (format == "gltf" ? true : false );
	}
	else
	{
		// TODO : process with other formats.
	}
}
#endif

Engine::Part::Part(const aiNode* node, const aiScene* scene, uint8_t indicesOfOneFace, std::string& assetDirectory, std::string& format) :
	indicesPerFace(indicesOfOneFace)
{
	objects.reserve(node->mNumMeshes);
	for (uint32_t index = 0; index < node->mNumMeshes; index++)
	{
		uint32_t meshIndex = node->mMeshes[index];
		aiMesh* mesh = scene->mMeshes[meshIndex];
		ProcessMesh(mesh, scene, assetDirectory, format, node);
	}

	name = node->mName.C_Str();
}

void Engine::Part::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string& assetDirectory, std::string& format, const aiNode* node)
{
	if (!mesh->HasPositions())
		return;

	std::vector<Vertex> vertices(mesh->mNumVertices);
	
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
		
		vertices[count].tangent = vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
		vertices[count].textureSlot = 0.0f;
	}

	std::vector<uint32_t> indices(mesh->mNumFaces * indicesPerFace);
	
	for (uint32_t count = 0; count < mesh->mNumFaces; count++)
	{
		aiFace face = mesh->mFaces[count];
		for(int index = 0; index < indicesPerFace; index ++)
		{
			if (index < face.mNumIndices)
				indices[count * indicesPerFace + index] = face.mIndices[index];
			else
				indices[count * indicesPerFace + index] = 0;
		}
	}
	
	if (indicesPerFace >= 3)
		CalcMikkTSpaceTangents(vertices, indices, indicesPerFace);
	
	printf("Process Mesh: %s of Node: %s, data: %d vertices, %d faces, %d vertex indices\n", mesh->mName.C_Str(), node->mName.C_Str(), vertices.size(), mesh->mNumFaces, indices.size());
	
	objects.emplace_back(vertices.data(), indices.data(), static_cast<uint32_t>(vertices.size()), static_cast<uint32_t>(indices.size()), assetDirectory);
	fflush(stdout);
	
	/* Load textures */
	uint32_t materialIndex = mesh->mMaterialIndex;
	Object& object = objects.back();
		
	aiMaterial* material = nullptr;
	if (scene->mMaterials != nullptr && materialIndex < scene->mNumMaterials)
	{
		material = scene->mMaterials[materialIndex];
	}
	
	if(material != nullptr)
	{
		object.SetBlendMode(ProcessMeshBlendMode(material));
		
		if (object.GetBlendMode() != BlendMode::Transparent)
		{
			float cutoff = 0.5f;
			material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, cutoff);
			object.m_Material.cutOff = cutoff;
		}
		
#ifdef PBR_PIPELINE
		LoadMeshTexturePBRPipeline(material, object, format, assetDirectory);
#elif defined(BLING_PHONG_PIPELINE)
		LoadMeshTexturesBlingPhongPipeline(material, object, format, assetDirectory);
#endif
	}
	else
	{
		object.SetBlendMode(BlendMode::Opaque);
	}
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

Engine::Model::Model() :
	m_ModelPath(""),
	m_Transform(GenerateModelMatrix(Transform())),
	m_NormalMatrix(glm::transpose(glm::inverse(glm::mat3(m_Transform))))
{
	
}

Engine::Model::Model(const std::string& path, bool bFlipUV, const Transform& transform) :
	m_ModelPath(path),
	m_Transform(GenerateModelMatrix(transform)),
	m_NormalMatrix(glm::transpose(glm::inverse(glm::mat3(m_Transform))))
{
	printf("Loading ...\n");
	Assimp::Importer importer;
	// importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	// uint32_t importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_SortByPType;
	uint32_t importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
	
	if (bFlipUV)
		importFlags |= aiProcess_FlipUVs;
	
	const aiScene* scene = importer.ReadFile(path, importFlags);
	
	uint32_t nodeNum = 1;
	GetChildrenNum(scene->mRootNode, nodeNum);	
	m_Parts.reserve(nodeNum);

	std::string assetDirectory = path.substr(0, path.find_last_of('/') + 1);
	std::string format = path.substr(path.find_last_of('.') + 1);
	ProcessNode(scene->mRootNode, scene, glm::mat4{1.0f}, assetDirectory, format);

	printf("Loading completed\n");
}

Engine::Model& Engine::Model::operator()(const std::string& path, bool bFlipUV, const Transform& transform)
{
	m_ModelPath = path;
	m_Transform = GenerateModelMatrix(transform);
	m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Transform)));
	
	printf("Loading ...\n");
	Assimp::Importer importer;
	// importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	// uint32_t importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_SortByPType;
	uint32_t importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
	
	if (bFlipUV)
		importFlags |= aiProcess_FlipUVs;
	
	const aiScene* scene = importer.ReadFile(path, importFlags);
	
	uint32_t nodeNum = 1;
	GetChildrenNum(scene->mRootNode, nodeNum);	
	m_Parts.reserve(nodeNum);

	std::string assetDirectory = path.substr(0, path.find_last_of('/') + 1);
	std::string format = path.substr(path.find_last_of('.') + 1);
	ProcessNode(scene->mRootNode, scene, glm::mat4{1.0f}, assetDirectory, format);

	printf("Loading completed\n");
	
	return *this;
}

void Engine::Model::ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform, std::string& assetDirectory, std::string& format)
{
	glm::mat4 localMat = aiMatrix4x4ToGlm(node->mTransformation);
	glm::mat4 globalMat = parentTransform * localMat;
	m_Parts.emplace_back(node, scene, 3, assetDirectory, format);
	m_Parts.back().localTransform = globalMat;
	
	for (uint32_t count = 0; count < node->mNumChildren; count++)
		ProcessNode(node->mChildren[count], scene, globalMat, assetDirectory, format);
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

#ifdef PBR_PIPELINE
void Engine::Model::BindAlbedoSlot(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindAlbedoSlots(slots, slotsNum);
}

void Engine::Model::BindMetallicSlots(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindMetallicSlots(slots, slotsNum);
}


void Engine::Model::BindRoughnessSlots(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindRoughnessSlots(slots, slotsNum);
}

void Engine::Model::BindAOSlots(int* slots, int slotsNum)
{
	for (auto& part : m_Parts)
		for (auto& object : part.objects)
			object.m_Material.BindAOSlots(slots, slotsNum);
}

#elif defined(BLING_PHONG_PIPELINE)

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

#endif

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