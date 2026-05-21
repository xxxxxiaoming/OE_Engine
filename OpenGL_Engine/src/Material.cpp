#include "Material.h"

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
        ambient[index] = slots[index];
}

void Engine::Material::BindDiffuseSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        diffuse[index] = slots[index];
}

void Engine::Material::BindSpecularSlots(int* slots, int slotsNum)
{
    slotsNum = slotsNum > MAX_TEXTURES ? MAX_TEXTURES : slotsNum;
    for (int index = 0; index < slotsNum; index++)
        specular[index] = slots[index];
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

