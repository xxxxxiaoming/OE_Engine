#pragma once
#include <vector>
#include <string>
#include "Shader.h"
#include "Texture.h"

#define MAX_TEXTURES 16

namespace Engine
{
	struct Material
	{
		Shader m_Shader;
		std::vector<Texture> m_Texture;
		
		Material(const std::string& vsPath, const std::string& fsPath, const std::string* textures, const int* textureSlots, const int textureNum);
		~Material();

		void Delete();

		void UseMaterial();
		void UnuseMaterial();

		inline Shader& GetShader() { return m_Shader; }
		
	private:
		bool m_Cleared = false;
	};
}
