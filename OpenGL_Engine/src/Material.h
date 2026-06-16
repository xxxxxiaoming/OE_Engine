#pragma once

#include "Shader.h"
#include "EngineConfig.h"
#include "Type.h"

namespace Engine
{
	#define MAX_TEXTURES (4) /* Support maximum 4 textures of each type in one material (just my decision.) */
	
	struct Material
	{
		Shader* shader = nullptr;
#ifdef PBR_PIPELINE
		// 不同纹理所使用的槽位
		int albedo[MAX_TEXTURES];
		int roughness[MAX_TEXTURES];
		int metallic[MAX_TEXTURES];
		int ao[MAX_TEXTURES];
		int transmission[MAX_TEXTURES];
		
		float metallicFactor = 0.0f;
		float roughnessFactor = 0.5f;
		float transmissionFactor = 0.0f;
		float cutOff = 0.5f;
		float ior = 1.5f;
		vec4 albedoFactor{1.0f, 1.0f, 1.0f, 1.0f};
		
		void BindAlbedoSlots(int* slots, int slotsNum);
		void BindRoughnessSlots(int* slots, int slotsNum);
		void BindMetallicSlots(int* slots, int slotsNum);
		void BindAOSlots(int* slots, int slotsNum);
		void BindTransmissionSlots(int* slots, int slotsNum);
		
#elif defined(BLING_PHONG_PIPELINE)
		
		int ambient[MAX_TEXTURES]{};
		int diffuse[MAX_TEXTURES]{}; // texture slot(diffuse) aka 这个材质对象使用的diffuse纹理插槽
		int specular[MAX_TEXTURES]{};// texture slot(specular) aka 这个材质对象使用的specular纹理插槽
		
		uint32_t shininess = 8;
		
		void BindAmbientSlots(int* slots, int slotsNum);
		void BindDiffuseSlots(int* slots, int slotsNum);
		void BindSpecularSlots(int* slots, int slotsNum);
		
		int GetTextureDiffuseSlot(int index);
		int GetTextureSpecularSlot(int index);
		
		void SetShininess(uint32_t shin) { shininess = shin; }
		
#endif
		
		int normal[MAX_TEXTURES]{};  // texture slot(normal) aka 这个材质对象使用的normal纹理插槽
		int emissive[MAX_TEXTURES]{};
		
		void UseMaterial()  const;
		void UnuseMaterial() const;
		
		void BindNormalSlots(int* slots, int slotsNum);
		void BindEmissiveSlots(int* slots, int slotsNum);

		void BindShader(Shader* sd) { shader = sd; }
		Shader* GetShader() { return shader; }
		
	};
}
