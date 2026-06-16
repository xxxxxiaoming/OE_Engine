#pragma once

#include "Engine.h"

#ifdef PBR_PIPELINE
    void CreatePBRMaterialSphere(Engine::Object& object);
    void CreatePBRScene(Engine::Object& floor, Engine::Model& model);
#endif

#ifdef BLING_PHONT_RENDERER
    void CreateAdvancedLightingScene(Engine::Object& floor, Engine::Object& wall, Engine::Object& glass, Engine::Model& model);
#endif
