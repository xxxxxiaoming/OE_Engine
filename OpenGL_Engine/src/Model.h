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

		bool m_Destroyed = false;

		// TODO: Link parts?
		// void LinkParts();
		void ProcessNode(const aiNode* node, const aiScene* scene, std::string& assetDirectroy);
		void GetChildrenNum(aiNode* node, uint32_t& count);

	public:
		Model(const std::string& path);
		~Model();

		void Destroy();

		void BindShader(Shader* shader);
		void BindDiffuseSlot(int* slots, int slotsNum);
		void BindSpecularSlot(int* slots, int slotsNum);
		void Draw(const Renderer& renderer);
		
		void EnableLight();
		void DisableLight();
	};
}
