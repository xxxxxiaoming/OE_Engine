#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "Object.h"
#include "Renderer.h"

namespace Engine
{
	/* A part is a node in assimp */
	struct Part
	{
		std::string name;
		std::vector<Object> objects;
		uint8_t indicesPerFace;

		Part(const aiNode* node, const aiScene* scene, uint8_t indicesOfOneFace, std::string& assetDirectory);
		~Part();
		
		void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string& assetDirectory);
		void DestroyPart();
	private:
		bool m_Destroyed = false;
	};
	
	class Model
	{
	private:
		std::string m_ModelPath;
		std::vector<Part> m_Parts;
		glm::mat4 m_Transform;
		glm::mat3 m_NormalMatrix;

		bool m_Destroyed = false;

		// TODO: Link parts?
		// void LinkParts();
		void ProcessNode(const aiNode* node, const aiScene* scene, std::string& assetDirectroy);
		void GetChildrenNum(aiNode* node, uint32_t& count);

	public:
		uint32_t m_VBOIntanced = 0;
		
		Model(const std::string& path, bool FlipUV = true, const Transform& transform = Transform());
		~Model();

		void Destroy();

		void BindShader(Shader* shader);
		void BindAmbientSlot(int* slots, int slotsNum);
		void BindDiffuseSlot(int* slots, int slotsNum);
		void BindSpecularSlot(int* slots, int slotsNum);
		void Draw(const Renderer& renderer);
		void DrawInstanced(const Renderer& renderer, uint32_t amount);
		
		void BindInstancedVertexAttrib(int index, int size, int type, int stride, int offset, uint32_t divisor);
		
		void EnableLight();
		void DisableLight();
		
		size_t GetPartsCount() const;
		size_t GetObjectsCount() const;
		glm::mat4 GetTransform() const { return m_Transform; }
		glm::mat3 GetNormalMatrix() const { return m_NormalMatrix; }
		
		void SetTransform(const Transform& transform)
		{
			m_Transform = GenerateModelMatrix(transform);
			m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Transform)));
		}
	};
}
