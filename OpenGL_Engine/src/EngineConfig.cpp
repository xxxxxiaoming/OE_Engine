#include "EngineConfig.h"

namespace Engine
{
    namespace Config
    {
        /*
        
        1 ~ 3
        shaderLight u_PointLightsDepthMap

        4
        shaderLight u_ShadowDepthMap

        5
        shaderLight u_PositionMap

        6 
        shaderLight u_LightSpacePositionMap

        7 
        shaderLight u_NormalMap

        8
        shader Light u_AlbedoMap

        9
        shaderLight u_SSAOMap

        10
        shaderLight u_MetallicRoughnessAOMap

        11
        shaderLight u_ShadowDepthMap

        12
        SSAO u_NoiseMap

        13 ~ 19
        Object albedo / metallic / roughness / ao / normal / transmission
        
        20
        ShaderLight u_EmissiveMap
        
        21
        shaderLightForward background
        
        22
        shaderLight irradiance
        
        23
        shaderLight radiance
        
        24
        shaderLight brdf_lut
        
        25
        Skybox cubemap
        */
        extern const uint8_t ENGINE_RESERVE_TEXTURES_SLOT_NUM = 26;
    }
}