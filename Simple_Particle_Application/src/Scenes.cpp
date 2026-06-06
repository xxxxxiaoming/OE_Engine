#include "Scenes.h"

void CreateAdvancedLightingScene(Engine::Object& floor, Engine::Object& wall, Engine::Object& glass, Engine::Model& model)
{
    // floor
    constexpr float sizeX = 700.0f;
    constexpr float sizeY = 700.0f;
    Engine::Vertex floorVertices[4];
    Engine::vec3 positions[1] = {
        Engine::vec3{-sizeX / 2.0f, -sizeY / 2.0f, 0.0f}
    };
	
    float width[1] = {
        sizeX
    };
    float height[1] = {
        sizeY
    };
    
    uint32_t floorIndices[6];
    
    std::string assetPath{"res/"};
    Engine::createRectangle(positions, width, height, floorVertices, floorIndices);
    
    for (int index = 0 ; index < 4 ; index++)
    {
        Engine::Vertex& vertex = floorVertices[index];
        vertex.texCoord.x *= 10;
        vertex.texCoord.y *= 10;
    }
	
    Engine::Transform floorTransform = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, -140.0f),
        glm::vec3(-90.0f, 0.0f, 0.0f)
    };
    floor(floorVertices, floorIndices, 4, 6, assetPath, floorTransform);
	
    floor.m_TexturesAmbient.reserve(1);
    floor.m_TexturesDiffuse.reserve(1);
    floor.m_TextureSpecular.reserve(1);
    floor.m_TexturesAmbient.emplace_back("res/texture/brickwall.jpg");
    floor.m_TexturesDiffuse.emplace_back("res/texture/brickwall.jpg");
    floor.m_TextureSpecular.emplace_back(0xFF000000);
    floor.m_TextureNormal.emplace_back("res/texture/brickwall_normal.jpg");
	
    int floorDiffuseSlot[1] = {1};
    int floorSpecularSlot[1] = {2};
    int floorNormalSlot[1] = {3};
    floor.m_Material.BindAmbientSlots(floorDiffuseSlot, 1);
    floor.m_Material.BindDiffuseSlots(floorDiffuseSlot, 1);
    floor.m_Material.BindSpecularSlots(floorSpecularSlot, 1);
    floor.m_Material.BindNormalSlots(floorNormalSlot, 1);
    floor.EnableLight();
    floor.SetBlendMode(Engine::BlendMode::Opaque);
    
    // wall
    constexpr float sizeXWall = 700.0f;
    constexpr float sizeYWall = 400.0f;
    Engine::Vertex wallVertices[4];
    Engine::vec3 wallPositions[1] = {
        Engine::vec3{-sizeX / 2.0f, -sizeY / 2.0f, 0.0f}
    };
	
    float wallWidth[1] = {
        sizeXWall
    };
    float wallHeight[1] = {
        sizeYWall
    };
    
    uint32_t wallIndices[6];
    
    Engine::createRectangle(wallPositions, wallWidth, wallHeight, wallVertices, wallIndices);
    
    for (int index = 0 ; index < 4 ; index++)
    {
        Engine::Vertex& vertex = wallVertices[index];
        vertex.texCoord.x *= 10;
        vertex.texCoord.y *= 10;
    }
	
    Engine::Transform wallTransform = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, sizeYWall / 2.0f, -100.0f),
        glm::vec3(0.0f, 0.0f, 0.0f)
    };
    wall(wallVertices, wallIndices, 4, 6, assetPath, wallTransform);
	
    wall.m_TexturesAmbient.reserve(1);
    wall.m_TexturesDiffuse.reserve(1);
    wall.m_TextureSpecular.reserve(1);
    wall.m_TexturesAmbient.emplace_back("res/texture/brickwall.jpg");
    wall.m_TexturesDiffuse.emplace_back("res/texture/brickwall.jpg");
    wall.m_TextureSpecular.emplace_back(0xFF000000);
    wall.m_TextureNormal.emplace_back("res/texture/brickwall_normal.jpg");
	
    int wallDiffuseSlot[1] = {1};
    int wallSpecularSlot[1] = {2};
    int wallNormalSlot[1] = {3};
    wall.m_Material.BindAmbientSlots(wallDiffuseSlot, 1);
    wall.m_Material.BindDiffuseSlots(wallDiffuseSlot, 1);
    wall.m_Material.BindSpecularSlots(wallSpecularSlot, 1);
    wall.m_Material.BindNormalSlots(wallNormalSlot, 1);
    wall.EnableLight();
    wall.SetBlendMode(Engine::BlendMode::Opaque);
    
    // glass
    constexpr float sizeXGlass = 100.0f;
    constexpr float sizeYGlass = 100.0f;
    Engine::Vertex glassVertices[4];
    Engine::vec3 glassPositions[1] = {
        Engine::vec3{-sizeXGlass / 2.0f, -sizeYGlass / 2.0f, 0.0f}
    };
	
    float glassWidth[1] = {
        sizeXGlass
    };
    float glassHeight[1] = {
        sizeYGlass
    };
	
    uint32_t glassIndices[6];
	
    Engine::createRectangle(glassPositions, glassWidth, glassHeight, glassVertices, glassIndices);
	
    Engine::Transform glassTransform = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, 50.0f, -30.0f),
        glm::vec3(0.0f, 0.0f, 0.0f)
    };
    glass(glassVertices, glassIndices, 4, 6, assetPath, glassTransform);
	
    glass.m_TexturesAmbient.reserve(1);
    glass.m_TexturesDiffuse.reserve(1);
    glass.m_TextureSpecular.reserve(1);
    glass.m_TexturesAmbient.emplace_back("res/texture/blending_transparent_window.png");
    glass.m_TexturesDiffuse.emplace_back("res/texture/blending_transparent_window.png");
    glass.m_TextureSpecular.emplace_back(0xFF000000);
    glass.m_TextureNormal.emplace_back(0xFFFF8080);
	
    int glassDiffuseSlot[1] = {8};
    int glassSpecularSlot[1] = {9};
    int glassNormalSlot[1] = {10};
    glass.m_Material.BindAmbientSlots(glassDiffuseSlot, 1);
    glass.m_Material.BindDiffuseSlots(glassDiffuseSlot, 1);
    glass.m_Material.BindSpecularSlots(glassSpecularSlot, 1);
    glass.m_Material.BindNormalSlots(glassNormalSlot, 1);
    glass.EnableLight();
    glass.SetBlendMode(Engine::BlendMode::Transparent);
    
    Engine::Transform nanosuitTransform = {
        glm::vec3(10.0f, 10.0f, 10.0f),
        glm::vec3(0.0f, 0.0f, -70.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
    };
   model("res/assets/nanosuit_reflection/nanosuit.obj", false
        , nanosuitTransform);
	
    int modelAmbientSlot[1] = {4};
    int modelDiffuseSlot[1] = {5};
    int modelSpecularSlot[1] = {6};
    int modelNormalSlot[1] = {7};
    model.BindAmbientSlot(modelAmbientSlot, 1);
    model.BindDiffuseSlot(modelDiffuseSlot, 1);
    model.BindSpecularSlot(modelSpecularSlot, 1);
    model.BindNormalSlot(modelNormalSlot, 1);
}