#include "Material.h"
#include "EngineConfig.h"

void Engine::Material::UseMaterial() const
{
    if (shader != nullptr && shader->CheckShaderValidity())
        shader->Use();
}

void Engine::Material::UnuseMaterial() const
{
    if (shader != nullptr && shader->CheckShaderValidity())
        shader->UnUse();
}

#ifdef PBR_PIPELINE
void Engine::Material::BindAlbedoSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        albedo[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindMetallicSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        metallic[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindRoughnessSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        roughness[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindAOSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        ao[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindTransmissionSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        transmission[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

#elif defined(BLING_PHONG_PIPLINE)
void Engine::Material::BindAmbientSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        ambient[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindDiffuseSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        diffuse[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindSpecularSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        specular[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

int Engine::Material::GetTextureDiffuseSlot(int index)
{
    assert(index >= 0 && index < MAX_TEXTURES);

    return diffuse[index];
}

int Engine::Material::GetTextureSpecularSlot(int index)
{
    assert(index >= 0 && index < MAX_TEXTURES);

    return specular[index];
}

#endif

void Engine::Material::BindNormalSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        normal[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

void Engine::Material::BindEmissiveSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        emissive[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
}

