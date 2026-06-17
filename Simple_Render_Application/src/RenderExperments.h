#pragma once
#include <string>
#include "Engine.h"

#ifdef PBR_PIPELINE
    
void PBRLighting(Engine::Renderer& renderer);

#elif defined(BLING_PHONT_PIPELINE)

    void StencilTestExperiment(const std::string& path, Engine::Renderer& renderer);
    void RenderTargetExperiment(Engine::Renderer& renderer);
    void DrawASimpleHouseUsingGS(Engine::Renderer& renderer);
    void InstanceExperiment(Engine::Renderer& renderer);
    void AdvancedLighting(Engine::Renderer& renderer);
    
#endif
