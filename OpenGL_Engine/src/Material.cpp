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

void Engine::Material::BindNormalSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        normal[index] = slots[index] + Engine::Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
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

