#pragma once
#include <vector>
#include <string>
#include "Shader.h"
#include "Texture.h"

namespace Engine
{
	#define MAX_TEXTURES (4) /* Support maximum 4 textures of each type in one material (just my decision.) */
	
	struct Material
	{
		Shader* shader = nullptr;
		int ambient[MAX_TEXTURES]{};
		int diffuse[MAX_TEXTURES]{}; // texture slot(diffuse) aka 这个材质对象使用的diffuse纹理插槽
		int specular[MAX_TEXTURES]{};// texture slot(diffuse) aka 这个材质对象使用的specular纹理插槽
		uint32_t shininess = 8;
		
		void UseMaterial()  const;
		void UnuseMaterial() const;
		void BindAmbientSlots(int* slots, int slotsNum);
		void BindDiffuseSlots(int* slots, int slotsNum);
		void BindSpecularSlots(int* slots, int slotsNum);

		inline void BindShader(Shader* sd) { shader = sd; }
		inline Shader* GetShader() { return shader; }
		int GetTextureDiffuseSlot(int index);
		int GetTextureSpecularSlot(int index);

		inline void SetShininess(uint32_t shin) { shininess = shin; }
	};
}
