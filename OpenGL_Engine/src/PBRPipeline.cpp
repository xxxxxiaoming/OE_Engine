#include "PBRPipeline.h"

#include "PBRVS.h"
#include "PBRVSDR.h"
#include "PBRFS.h"
#include "PBRFSDR.h"
#include "PLFS_SSAO.h"
#include "Random.h"

#include <gli.hpp>
#include <iostream>
#include <stb_image.h>

static const int MAX_NON_DIRECTIONAL_LIGHTS = 4;
static const int SSAO_SAMPLES_NUMBER = 64;
static const int SSAO_NOISE_NUMBER = 16;

static void LoadIBLTexture(const std::string& irradiance, const std::string& radiance, const std::string& brdfPath, uint32_t& tIR, uint32_t& tR, uint32_t& brdf)
{
    // ==========================================
    // 助手函数 1：用 GLI 加载 DDS 立方体贴图
    // ==========================================
    auto loadCubemapDDS = [](const std::string& path) -> uint32_t {
        gli::texture_cube texture(gli::load(path));
        if (texture.empty()) {
            std::cerr << "[GLI Error] IBL 立方体贴图加载失败: " << path << std::endl;
            return 0;
        }

        gli::gl GL(gli::gl::PROFILE_GL33);
        gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());

        uint32_t texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

        for (std::size_t face = 0; face < 6; ++face) {
            for (std::size_t level = 0; level < texture.levels(); ++level) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                    level, format.Internal,
                    texture[face][level].extent().x, texture[face][level].extent().y,
                    0, format.External, format.Type,
                    texture[face][level].data()
                );
            }
        }
        
        if (texture.levels() == 1) {
            // 如果只有 1 层 (比如 Irradiance 漫反射图)，就不使用 Mipmap 过滤
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            // 如果有多层 (比如 Radiance 镜面反射图)，使用 Mipmap 过滤
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, texture.levels() - 1);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texID;
    };

    // ==========================================
    // 助手函数 2：用 stb_image 加载 2D PNG 贴图
    // ==========================================
    auto loadBRDFLUT = [](const std::string& path) -> uint32_t {
        int width, height, nrChannels;
        
        // 注意：如果你之前全局设置了 stbi_set_flip_vertically_on_load(true)，
        // 这张图也会被翻转，通常 PBR 渲染不影响，但如果高光边缘奇怪，可以尝试在这里临时设为 false
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        
        if (!data) {
            std::cerr << "[STB Error] BRDF LUT 加载失败: " << path << std::endl;
            return 0;
        }

        uint32_t texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // 致命设置：防止粗糙度为 1.0 时采样越界产生亮边
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return texID;
    };

    // ==========================================
    // 执行加载并赋值
    // ==========================================
    tIR  = loadCubemapDDS(irradiance);
    tR   = loadCubemapDDS(radiance);
    brdf = loadBRDFLUT(brdfPath);
    
    std::cout << "[IBL System] 所有资源加载完毕!\n"
              << " -> Irradiance ID : " << tIR << "\n"
              << " -> Radiance ID   : " << tR << "\n"
              << " -> BRDF LUT ID   : " << brdf << std::endl;
}

static void GenerateSSAOSamples(std::vector<glm::vec3>& samples, uint32_t& noiseTexture)
{
    auto func = [](int i)->float
    {
        float scale = static_cast<float>(i) / SSAO_SAMPLES_NUMBER;
        scale = 0.1f + scale * scale * (1.0f - 0.1f);
        return scale;
    }; 
    samples.reserve(SSAO_SAMPLES_NUMBER);
    Engine::Random::Init();
    for(int count = 0; count < SSAO_SAMPLES_NUMBER; count++)
    {
        glm::vec3 sample{
            Engine::Random::Float(-1.0f, 1.0f),
            Engine::Random::Float(-1.0f, 1.0f),
            Engine::Random::Float(0.0f, 1.0f)
        }; // sample 这时候是一个[-1,1] * [-1,1] * [0,1]的长方体
        
        sample = glm::normalize(sample); // 这个时候才是一个（半径为1）半球体哦。
        float scale = func(count);
        samples.emplace_back(sample * scale);
    }
    
    std::vector<glm::vec3> noise;
    noise.reserve(SSAO_NOISE_NUMBER);
    
    for (size_t count = 0; count < SSAO_NOISE_NUMBER; count++)
    {
        noise.emplace_back(Engine::Random::Float(-1.0f, 1.0f), Engine::Random::Float(-1.0f, 1.0f), Engine::Random::Float(-1.0f, 1.0f));
    }
    
    GLCALL(glGenTextures(1, &noiseTexture));
    GLCALL(glBindTexture(GL_TEXTURE_2D, noiseTexture));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise.data()));
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void Engine::PBRPipeline::UnblockModelInternal(Model* model)
{
    std::vector<Part>& allParts = model -> GetParts();
    for (auto& part : allParts)
    {   
        int index = 0;
        for (auto& object : part.objects)
        {
            UnblockObjectInternal(&object);
            index++;
        }
    }
}

void Engine::PBRPipeline::UnblockObjectInternal(Object* object)
{
    switch (object->GetBlendMode())
    {
    case BlendMode::Opaque :
    case BlendMode::Masked :
        object->m_Material.shader = &m_ShaderGBuffer;
        break;
    case BlendMode::Transparent:
        object->m_Material.shader = &m_ShaderLightForward;
        break;
    }
    
    object->EnableLight();
}

void Engine::PBRPipeline::GenerateShadowMapInternal(Renderer& renderer)
{
    m_ShadowMap.OnCapture(renderer);
    
    for (auto& objectPair : m_OpaqueObjects)
    {
        Object* object = objectPair.second;
        if (object != nullptr)
        {
            BlockObjectInternal(object);
            m_ShadowMap.CaptureObject(object->GetTransform(), *object, renderer);
            UnblockObjectInternal(object);
        }
    }
    
    for (auto& objectPair : m_MaskedObjects)
    {
        Object* object = objectPair.second;
        if (object != nullptr)
        {
            BlockObjectInternal(object);
            m_ShadowMap.CaptureObject(object->GetTransform(), *object, renderer);
            UnblockObjectInternal(object);
        }
    }
        
    m_ShadowMap.PostCapture(renderer);
    m_ShadowMap.SaveDepthMap("res/screenshot/depthmap.png");
}

void Engine::PBRPipeline::GenerateShadowMapPointInternal(Renderer& renderer)
{
    for (int index = 0; index < m_PointLightIndex; index++)
    {
        GenerateSpecificShadowMapPointInternal(index, renderer);
    }
}

void Engine::PBRPipeline::GenerateSpecificShadowMapPointInternal(int index, Renderer& renderer)
{
    ShadowMapPoint& shadowMapPoint = m_ShadowMapPoint[index];
    shadowMapPoint.OnCapture(renderer);
        
    for (auto& objectPair : m_OpaqueObjects)
    {
        Object* object = objectPair.second;
        if (object != nullptr)
        {
            BlockObjectInternal(object);
            shadowMapPoint.CaptureObject(object->GetTransform(), *object, renderer);
            UnblockObjectInternal(object);
        }
    }
    
    for (auto& objectPair : m_MaskedObjects)
    {
        Object* object = objectPair.second;
        if (object != nullptr)
        {
            BlockObjectInternal(object);
            shadowMapPoint.CaptureObject(object->GetTransform(), *object, renderer);
            UnblockObjectInternal(object);
        }
    }
    
    // std::string savePath = "res/screenshot/depthCubeMap" + std::to_string(index) + std::string{'/'};
    shadowMapPoint.PostCapture(renderer);
    // shadowMapPoint.SaveDepthMap(savePath);
    
    m_PointLightsNeedCapture[index] = false;
    
    GLCALL(glBindTextureUnit(1 + index, shadowMapPoint.GetShadowMap()));
}

/*
 * @brief 管线中的前向渲染部分，只渲染透明物体，所有透明物体都会被放到这个流程中渲染
 */
void Engine::PBRPipeline::ForwardRenderInternal(const Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    m_RTLight.BindFramebuffer();
    m_ShaderLightForward.Use();
    m_ShaderLightForward.SetUniform1i("u_Material.background", 21);
    
    renderer.EnableBlend();
    GLCALL(glDepthMask(GL_FALSE));
    GLCALL(glBindTextureUnit(21, m_FRBackgroundTexture));
    
    std::vector<Object*> transparentObjects;
    transparentObjects.reserve(m_TransparentObjects.size());
    glm::vec3& camPostiion = m_CameraPosition;
    for (auto& objectPair : m_TransparentObjects)
    {
        transparentObjects.emplace_back(objectPair.second);
    }
    std::sort(transparentObjects.begin(), transparentObjects.end(),
        [camPostiion](const Object* objectA, const Object* objectB)->bool
        {
            float distanceA = glm::distance(objectA->GetPosition(), camPostiion);
            float distanceB = glm::distance(objectB->GetPosition(), camPostiion);
            
            return distanceA > distanceB;
        });
        
    // for (auto& objectPair : m_TransparentObjects)
    for (auto object : transparentObjects)
    {
        // Object* object = objectPair.second;
        if (object != nullptr)
        {
            ConfigMVPMatrix(object->GetTransform(), viewMatrix, projectionMatrix);
            ConfigNormalMatrix(object->GetNormalMatrix());

            object->OnDraw();
            renderer.DrawElements(object->GetIndexCount(), nullptr);
        }
    }
    
    renderer.DisableBlend();
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glBindTextureUnit(21, 0));
    
    m_ShaderLightForward.UnUse();
    m_RTLight.UnbindFramebuffer();
}

void Engine::PBRPipeline::DeferredRenderInternal(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    // Generate g-buffer
    m_ShaderGBuffer.Use();
    m_RTGBuffer.BindFramebuffer();
    
    renderer.OnRender();
    renderer.DisableBlend();
    
    for (auto& objectPair : m_OpaqueObjects)
    {
        Object* object = objectPair.second;
        if (object != nullptr)
        {
            ConfigMVPMatrix(object->GetTransform(), viewMatrix, projectionMatrix);
            ConfigNormalMatrix(object->GetNormalMatrix());
            
            
            object->OnDraw();
            renderer.DrawElements(object->GetIndexCount(), nullptr);
        }
    }
    
    for (auto& objectPair : m_MaskedObjects)
    {
        Object* object = objectPair.second;
        if (object != nullptr)
        {
            ConfigMVPMatrix(object->GetTransform(), viewMatrix, projectionMatrix);
            ConfigNormalMatrix(object->GetNormalMatrix());
            
            
            object->OnDraw();
            renderer.DrawElements(object->GetIndexCount(), nullptr);
        }
    }
    
    renderer.EnableBlend();
    
    m_ShaderGBuffer.UnUse();
    m_RTGBuffer.UnbindFramebuffer();
    
    uint32_t positionBuffer = m_RTGBuffer.GetTextureBuffer(0);
    uint32_t lightSpacePosBuffer = m_RTGBuffer.GetTextureBuffer(1);
    uint32_t normalBuffer = m_RTGBuffer.GetTextureBuffer(2);
    uint32_t albedoBuffer = m_RTGBuffer.GetTextureBuffer(3);
    uint32_t metallicRoughnessAOBuffer = m_RTGBuffer.GetTextureBuffer(4);
    uint32_t emissiveBuffer = m_RTGBuffer.GetTextureBuffer(5);
    
    GLCALL(glBindTextureUnit(5, positionBuffer));
    GLCALL(glBindTextureUnit(6, lightSpacePosBuffer));
    GLCALL(glBindTextureUnit(7, normalBuffer));
    GLCALL(glBindTextureUnit(8, albedoBuffer));
    GLCALL(glBindTextureUnit(10, metallicRoughnessAOBuffer));
    GLCALL(glBindTextureUnit(20, emissiveBuffer));
    
    m_ShaderSSAO.Use();
    m_RTSSAO.BindFramebuffer();
    
    m_ShaderSSAO.SetUniform1i("u_PositionMap", 5);
    m_ShaderSSAO.SetUniform1i("u_NormalMap", 7);
    
    renderer.OnRender();
    
    m_RenderRect.OnDraw();
    
    renderer.DrawElements(m_RenderRect.GetIndexCount(), nullptr);
    
    m_ShaderSSAO.UnUse();
    m_RTSSAO.UnbindFramebuffer();
    
    m_ShaderSSAOSmoth.Use();
    m_RTSSAOSmoth.BindFramebuffer();
    
    uint32_t ssaoTexture = m_RTSSAO.GetTextureBuffer();
    
    GLCALL(glBindTextureUnit(9, ssaoTexture));
    m_ShaderSSAOSmoth.SetUniform1i("u_SSAOMap", 9);
    
    renderer.OnRender();
    
    m_RenderRect.OnDraw();
    
    renderer.DrawElements(m_RenderRect.GetIndexCount(), nullptr);
    
    
    m_ShaderSSAOSmoth.UnUse();
    m_RTSSAOSmoth.UnbindFramebuffer();
    
    uint32_t ssaoSmoothTexture = m_RTSSAOSmoth.GetTextureBuffer();
    GLCALL(glBindTextureUnit(9, ssaoSmoothTexture));
    
    // Generate final frame
    m_ShaderLight.Use();
    m_RTLight.BindFramebuffer();

    m_ShaderLight.SetUniform1i("u_PositionMap", 5);
    m_ShaderLight.SetUniform1i("u_LightSpacePositionMap", 6);
    m_ShaderLight.SetUniform1i("u_NormalMap", 7);
    m_ShaderLight.SetUniform1i("u_AlbedoMap", 8);
    m_ShaderLight.SetUniform1i("u_SSAOMap", 9);
    m_ShaderLight.SetUniform1i("u_MetallicRoughnessAOMap", 10);
    m_ShaderLight.SetUniform1i("u_EmissiveMap", 20);
    
    renderer.OnRender();
    
    m_RenderRect.OnDraw();
    
    renderer.DrawElements(m_RenderRect.GetIndexCount(), nullptr);
    
    m_ShaderLight.UnUse();
    m_RTLight.UnbindFramebuffer();
}

Engine::PBRPipeline::PBRPipeline(uint16_t renderResolutionX, uint16_t renderResolutionY, int shadowMapResolution, bool bDeffered) : 
    m_ShadowMap(shadowMapResolution, false),
    m_UseDeffered(bDeffered),
    m_ShadowMapResolution(shadowMapResolution),
    m_RTGBuffer(renderResolutionX, renderResolutionY, true),
    m_RTLight(renderResolutionX, renderResolutionY, true),
    m_RTSSAO(renderResolutionX, renderResolutionY, true),
    m_RTSSAOSmoth(renderResolutionX, renderResolutionY, true)
{
    m_ShadowMapPoint.reserve(MAX_NON_DIRECTIONAL_LIGHTS);
    m_PointLightsNeedCapture.reserve(MAX_NON_DIRECTIONAL_LIGHTS);
    
    for (int i = 0; i < MAX_NON_DIRECTIONAL_LIGHTS; i++)
        m_PointLightsNeedCapture.emplace_back(false);
    

    uint32_t colorAttachments[6] = {0,1,2,3,4,5};
        
    m_ShaderLightForward.CreateShaderFromSource(pbrVsCode, pbrFsCode);
    m_ShaderLight.CreateShaderFromSource(pbrVsLightCode, pbrFsLightCode);
    m_ShaderGBuffer.CreateShaderFromSource(pbrVsDRCode, pbrFsDRCode);
    m_ShaderSSAO.CreateShaderFromSource(pbrVsLightCode, fsSSAOCode);
    m_ShaderSSAOSmoth.CreateShaderFromSource(pbrVsLightCode, fsSSAOBlurCode);
    
    int resolution[2] = {renderResolutionX, renderResolutionY};
    std::vector<glm::vec3> samples;
    GenerateSSAOSamples(samples, m_SSAONoiseTexture);
    GLCALL(glBindTextureUnit(12, m_SSAONoiseTexture));
    m_ShaderSSAO.SetUniform3fv("u_Samples", samples.size(), samples.data());
    m_ShaderSSAO.SetUniform1iv("u_Resolution", 2, resolution);
    m_ShaderSSAO.SetUniform1i("u_NoiseMap", 12);
    
    // m_FRBackgroundTexture 用来在前向渲染中渲染透明物体，其实就是m_RTLight中的color buffer
    GLCALL(glGenTextures(1, &m_FRBackgroundTexture));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_FRBackgroundTexture));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, renderResolutionX, renderResolutionY, 0, GL_RGB, GL_FLOAT, nullptr));
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
        
    m_RTGBuffer.CreateColorAttachment(0);
    m_RTGBuffer.CreateColorAttachment(1);
    m_RTGBuffer.CreateColorAttachment(2);
    m_RTGBuffer.CreateColorAttachment(3, GL_RGBA);
    m_RTGBuffer.CreateColorAttachment(4, GL_RGBA); // MRA Map
    m_RTGBuffer.CreateColorAttachment(5);
    m_RTGBuffer.CreateRenderBuffer();
    m_RTGBuffer.UseMultiColorAttachments(colorAttachments, 6);
    
    m_RTSSAO.CreateColorAttachment(0, GL_RGB);
    m_RTSSAO.CreateRenderBuffer();
    
    m_RTSSAOSmoth.CreateColorAttachment(0, GL_RGB);
    m_RTSSAOSmoth.CreateRenderBuffer();
        
    m_RTLight.CreateColorAttachment(0);
    m_RTLight.CreateRenderBuffer();
        
    vec3 rectPos[1] = { vec3{-1.0f, -1.0f, 0.0f} };
    Vertex vertices[4];
    uint32_t indices[6];
    float widthHDRRect[1] = { 2.0f };
    float heightHDRRect[1] = { 2.0f };
    std::string assetDirector{ "res/texture" };
	
    createRectangle(rectPos, widthHDRRect, heightHDRRect, vertices, indices, 1);
    m_RenderRect(vertices, indices, 4, 6, assetDirector);
    m_RenderRect.DisableLight();
    
    
    /* 默认只开启方向光源 */
    m_ShaderLight.SetUniform1i("u_LightConfig.enableDirectionalLight", 1);	// 开启方向光源
    m_ShaderLight.SetUniform1i("u_LightConfig.pointLightNum", 0);			// 不使用点光源
    m_ShaderLight.SetUniform1i("u_LightConfig.spotLightNum", 0);			// 不使用聚光源
    m_ShaderLight.SetUniform1i("u_LightConfig.enableIBL", 0);              // 不使用IBL
    
    /* 方向光配置 */
    m_ShaderLight.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color", 1.0f, 1.0f, 1.0f);
    
     /* 默认只开启方向光源 */
    m_ShaderLightForward.SetUniform1i("u_LightConfig.enableDirectionalLight", 1);	// 开启方向光源
    m_ShaderLightForward.SetUniform1i("u_LightConfig.pointLightNum", 0);			// 不使用点光源
    m_ShaderLightForward.SetUniform1i("u_LightConfig.spotLightNum", 0);			// 不使用聚光源
        
    /* 方向光配置 */
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color", 1.0f, 1.0f, 1.0f);
    
}

void Engine::PBRPipeline::TurnOn(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    // Check if shadow map captured.
    for (int i = 0; i < MAX_NON_DIRECTIONAL_LIGHTS; i++)
    {
        if (m_PointLightsNeedCapture[i])
            GenerateSpecificShadowMapPointInternal(i, renderer);
    }
    
    if (!m_ShadowMapCaptured)
    {
        GenerateShadowMapInternal(renderer);
        m_ShadowMapCaptured = true;
    }
    
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity())
        return;
    
    uint32_t shadowMap = m_ShadowMap.GetShadowMap();
    GLCALL(glBindTextureUnit(11, shadowMap));
    m_ShaderLight.SetUniform1i("u_ShadowDepthMap", 11);

    DeferredRenderInternal(renderer, viewMatrix, projectionMatrix);
    
    int bufferSizeX, bufferSizeY;
    m_RTGBuffer.GetRTSize(bufferSizeX, bufferSizeY);
    
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RTGBuffer.GetFBO()));
    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTLight.GetFBO()));
    GLCALL(glBlitFramebuffer(0, 0, bufferSizeX, bufferSizeY, 0, 0, bufferSizeX, bufferSizeY, GL_DEPTH_BUFFER_BIT, GL_NEAREST));
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    
    GLCALL(glBindTextureUnit(0, m_FRBackgroundTexture));
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RTLight.GetFBO()));
    GLCALL(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, bufferSizeX, bufferSizeY));
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCALL(glBindTextureUnit(0, 0));
    
    ForwardRenderInternal(renderer, viewMatrix, projectionMatrix);
}

void Engine::PBRPipeline::TurnOff(Renderer& renderer) const
{
    if (m_UseDeffered && m_ShaderGBuffer.CheckShaderValidity())
        m_ShaderGBuffer.UnUse();
    
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.UnUse();
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.UnUse();
}

/* IBL Light */
void Engine::PBRPipeline::ConfigIBLLight(const std::string& irradiance, const std::string& radiance, const std::string& brdf)
{
    LoadIBLTexture(irradiance, radiance, brdf, m_IBLIrradianceTexture, m_IBLRadianceTexture, m_IBLBRDFLUT);
    GLCALL(glBindTextureUnit(22, m_IBLIrradianceTexture));
    GLCALL(glBindTextureUnit(23, m_IBLRadianceTexture));
    GLCALL(glBindTextureUnit(24, m_IBLBRDFLUT));
    
    if (m_ShaderLight.CheckShaderValidity())
    {
        m_ShaderLight.SetUniform1i("u_IBL.irradiance", 22);
        m_ShaderLight.SetUniform1i("u_IBL.radiance", 23);
        m_ShaderLight.SetUniform1i("u_IBL.BRDF_LUT", 24);
    }
    
    if (m_ShaderLightForward.CheckShaderValidity())
    {
        m_ShaderLightForward.SetUniform1i("u_IBL.irradiance", 22);
        m_ShaderLightForward.SetUniform1i("u_IBL.radiance", 23);
        m_ShaderLightForward.SetUniform1i("u_IBL.BRDF_LUT", 24);
    }
}

void Engine::PBRPipeline::EnableIBLLight()
{
    if (m_ShaderLight.CheckShaderValidity())
        m_ShaderLight.SetUniform1i("u_LightConfig.enableIBL", 1);
    
    if (m_ShaderLightForward.CheckShaderValidity())
        m_ShaderLightForward.SetUniform1i("u_LightConfig.enableIBL", 1);
}

void Engine::PBRPipeline::DisableIBLLight()
{
    if (m_ShaderLight.CheckShaderValidity())
        m_ShaderLight.SetUniform1i("u_LightConfig.enableIBL", 0);
    
    if (m_ShaderLightForward.CheckShaderValidity())
        m_ShaderLightForward.SetUniform1i("u_LightConfig.enableIBL", 0);
}

/* Directional Light */

void Engine::PBRPipeline::EnableDirectionLight()
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform1i("u_LightConfig.enableDirectionalLight", 1);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform1i("u_LightConfig.enableDirectionalLight", 1);
}

void Engine::PBRPipeline::DisableDirectionLight()
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform1i("u_LightConfig.enableDirectionalLight", 0);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform1i("u_LightConfig.enableDirectionalLight", 0);
}

void Engine::PBRPipeline::ConfigDirectionLight(const vec3& direction, const vec3& color)
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform3f("u_DirectionLight.direction", direction.x, direction.y, direction.z);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color", color.x, color.y, color.z);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.direction", direction.x, direction.y, direction.z);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color", color.x, color.y, color.z);
}

/* Point Light */

bool Engine::PBRPipeline::AddPointLight(const vec3& position, const vec3& color, float constant, float linear, float quadratic)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || m_PointLightIndex >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(m_PointLightIndex) + "].";
    std::string uniformNamePointDepthMap = "u_PointLightsDepthMap[" + std::to_string(m_PointLightIndex) + "]";
    
    m_ShaderLight.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLight.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLight.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLight.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLight.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    m_ShaderLight.SetUniform1i(uniformNamePointDepthMap, m_PointLightIndex + 1);
    m_ShaderLight.SetUniform1i("u_LightConfig.pointLightNum", ++m_PointLightIndex);
    
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLightForward.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLightForward.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLightForward.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    m_ShaderLightForward.SetUniform1i("u_LightConfig.pointLightNum", m_PointLightIndex);
    
    m_ShadowMapPoint.emplace_back(m_ShadowMapResolution, false);
    m_PointLightsNeedCapture[m_PointLightIndex - 1] = true;
    
    return true;
}

bool Engine::PBRPipeline::ConfigPointLightPosition(int index, const vec3& position)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShadowMapPoint[index].UpdateCapturePosition(glm::vec3{position.x, position.y, position.z});
    m_PointLightsNeedCapture[index] = true;
    
    return true;
}

bool Engine::PBRPipeline::ConfigPointLightColor(int index, const vec3& color)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    
    return true;
}

bool Engine::PBRPipeline::ConfigPointLightAttenuation(int index, float constant, float linear, float quadratic)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLight.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLight.SetUniform1f(uniformName + "quadratic", quadratic);
    
    m_ShaderLightForward.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLightForward.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLightForward.SetUniform1f(uniformName + "quadratic", quadratic);
    
    return true;
}

bool Engine::PBRPipeline::AddSpotLight(const vec3& position, const vec3& direction, const vec3& color, float innerAngle, float outterAngle, float constant, float linear, float quadratic)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || m_SpotLightIndex >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(m_SpotLightIndex) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLight.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_ShaderLight.SetUniform1f(uniformName + "innerAngle", glm::radians(innerAngle));
    m_ShaderLight.SetUniform1f(uniformName + "outterAngle", glm::radians(outterAngle));
    m_ShaderLight.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLight.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLight.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLight.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    m_ShaderLight.SetUniform1i("u_LightConfig.spotLightNum", ++m_SpotLightIndex);
    
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_ShaderLightForward.SetUniform1f(uniformName + "innerAngle", glm::radians(innerAngle));
    m_ShaderLightForward.SetUniform1f(uniformName + "outterAngle", glm::radians(outterAngle));
    m_ShaderLightForward.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLightForward.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLightForward.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLightForward.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    m_ShaderLightForward.SetUniform1i("u_LightConfig.spotLightNum", m_SpotLightIndex);
    
    return true;
}

bool Engine::PBRPipeline::ConfigSpotLightPosition(int index, const vec3& position)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    
    return true;
}

bool Engine::PBRPipeline::ConfigSpotLightDirection(int index, const vec3& direction)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    
    return true;
}

bool Engine::PBRPipeline::ConfigSpotLightColor(int index, const vec3& color)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color", color.x, color.y, color.z);
    
    return true;
}

bool Engine::PBRPipeline::ConfigSpotLightAttenuation(int index, float constant, float linear, float quadratic)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLight.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLight.SetUniform1f(uniformName + "quadratic", quadratic);
    
    m_ShaderLightForward.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLightForward.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLightForward.SetUniform1f(uniformName + "quadratic", quadratic);
    
    return true;
}

bool Engine::PBRPipeline::ConfitSpotLightScale(int index, float innerAngle, float outterAngle)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform1f(uniformName + "innerAngle", innerAngle);
    m_ShaderLight.SetUniform1f(uniformName + "outterAngle", outterAngle);
    
    m_ShaderLightForward.SetUniform1f(uniformName + "innerAngle", innerAngle);
    m_ShaderLightForward.SetUniform1f(uniformName + "outterAngle", outterAngle);
    
    return true;
}

void Engine::PBRPipeline::ConfigMVPMatrix(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    m_ShaderGBuffer.SetUniformMatrix4f("u_Model", modelMatrix);
    m_ShaderGBuffer.SetUniformMatrix4f("u_View", viewMatrix);
    m_ShaderGBuffer.SetUniformMatrix4f("u_Projection", projectionMatrix);   
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniformMatrix4f("u_Model", modelMatrix);
    m_ShaderLightForward.SetUniformMatrix4f("u_View", viewMatrix);
    m_ShaderLightForward.SetUniformMatrix4f("u_Projection", projectionMatrix);
    
    if (!m_ShaderSSAO.CheckShaderValidity())
        return;
    
    m_ShaderSSAO.SetUniformMatrix4f("u_View", viewMatrix);
    m_ShaderSSAO.SetUniformMatrix4f("u_Projection", projectionMatrix);
}

void Engine::PBRPipeline::ConfigNormalMatrix(const glm::mat3& normalMatrix)
{
    if (!m_ShaderGBuffer.CheckShaderValidity())
        return;
        
    m_ShaderGBuffer.SetUniformMatrix3f("u_NormalMat", normalMatrix);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniformMatrix3f("u_NormalMat", normalMatrix);
    
}

void Engine::PBRPipeline::ConfigCameraWorldPosition(const glm::vec3& position)
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform3f("u_CameraPosition", position.x, position.y, position.z);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform3f("u_CameraPosition", position.x, position.y, position.z);
    
    m_CameraPosition = position;
}

void Engine::PBRPipeline::ConfigShadowMapCaptureView(glm::vec3 capturePosition, glm::vec3 captureLookAt, float viewLeft, float viewRight, float viewBottom, float viewTop, float viewNear, float viewFar)
{
    Engine::Camera shadowMapCaptureCam{capturePosition, captureLookAt, glm::vec3(0.0f, 1.0f, 0.0f)};
    glm::mat4 projectionMatrix = glm::ortho(viewLeft, viewRight, viewBottom, viewTop, viewNear, viewFar);
    glm::mat4 lightSpace = projectionMatrix * shadowMapCaptureCam.GetViewMatrix();
    
    m_ShaderGBuffer.SetUniformMatrix4f("u_LightSpace", lightSpace);
    
    m_ShadowMap.SetCaptureView(shadowMapCaptureCam, projectionMatrix);
}

void Engine::PBRPipeline::ConfigShadowMapPointCaptureView(int index, const glm::vec3& captureWorldPosition, float fov, float aspect, float nearPlane, float farPlane)
{
    m_ShaderLight.SetUniform1f("u_FarPlane", farPlane);
    m_ShadowMapPoint[index].SetCaptureView(captureWorldPosition, fov, aspect, nearPlane, farPlane);
    m_PointLightsNeedCapture[index] = true;
}

void Engine::PBRPipeline::AddModel(const std::string& name, Model* model)
{
    // model -> BindShader(m_UseDeffered ? &m_ShaderGBuffer : &m_ShaderLight); 
    // model -> EnableLight();
            
    if (m_Models.find(name) == m_Models.end())
        m_Models[name] = model;
    
    std::vector<Part>& allParts = model -> GetParts();
    
    size_t partIndex = 0;
    size_t meshIndex = 0;
    
    for (auto& part : allParts)
    {   
        for (auto& object : part.objects)
        {
            std::string objectName = name + "_" + part.name + '.' + std::to_string(partIndex) + "_" + std::to_string(meshIndex);
            AddObject(objectName, &object);
            object.SetTransform(model->GetTransform() * part.localTransform);
            meshIndex++;
        }
        partIndex++;
    }
}
void Engine::PBRPipeline::AddObject(const std::string& name, Object* object)
{
    if (m_Objects.find(name) == m_Objects.end())
        m_Objects[name] = object;
    
    if (object->m_UseMRA)
    {
        int textureSlot[1] = {13 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM};
        object->m_Material.BindAlbedoSlots(textureSlot, 1);
        
        textureSlot[0] = 14 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindMetallicSlots(textureSlot, 1);
        object->m_Material.BindRoughnessSlots(textureSlot, 1);
        
        textureSlot[0] = 15 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindAOSlots(textureSlot, 1);
    
        textureSlot[0] = 16 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindNormalSlots(textureSlot, 1);
        
        textureSlot[0] = 17 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindEmissiveSlots(textureSlot, 1);
        
        if (object->GetBlendMode() == BlendMode::Transparent || object->GetBlendMode() == BlendMode::TransparentMasked)
        {
            // PBR管线中，透明物体需要多一张 transmission（不透明度） 纹理图
            textureSlot[0] = 18 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
            object->m_Material.BindTransmissionSlots(textureSlot, 1);
        }
    }
    else
    {
        int textureSlot[1] = {13 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM};
        object->m_Material.BindAlbedoSlots(textureSlot, 1);
    
        textureSlot[0] = 14 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindMetallicSlots(textureSlot, 1);
    
        textureSlot[0] = 15 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindRoughnessSlots(textureSlot, 1);
    
        textureSlot[0] = 16 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindAOSlots(textureSlot, 1);
    
        textureSlot[0] = 17 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindNormalSlots(textureSlot, 1);
        
        textureSlot[0] = 18 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
        object->m_Material.BindEmissiveSlots(textureSlot, 1);
        
        if (object->GetBlendMode() == BlendMode::Transparent || object->GetBlendMode() == BlendMode::TransparentMasked)
        {
            textureSlot[0] = 19 - Config::ENGINE_RESERVE_TEXTURES_SLOT_NUM;
            object->m_Material.BindTransmissionSlots(textureSlot, 1);
        }
    }

    object->EnableLight();
    switch (object->GetBlendMode())
    {
    case BlendMode::Opaque :
        if (m_OpaqueObjects.find(name) == m_OpaqueObjects.end())
        {
            object->m_Material.shader = &m_ShaderGBuffer;
            m_OpaqueObjects[name] = object;
        }
        else
        {
            printf("Opaque object exists! %s\n", name.c_str());
        }
        break;
    case BlendMode::Masked :
        if (m_MaskedObjects.find(name) == m_MaskedObjects.end())
        {
            object->m_Material.shader = &m_ShaderGBuffer;
            m_MaskedObjects[name] = object;
        }
        else
        {
            printf("Masked object exists! %s\n", name.c_str());   
        }
        break;
    case BlendMode::Transparent:
    case BlendMode::TransparentMasked:
        if (m_TransparentObjects.find(name) == m_TransparentObjects.end())
        {
            object->m_Material.shader = &m_ShaderLightForward;
            m_TransparentObjects[name] = object;
        }
        else
        {
            printf("Transparent object exists! %s\n", name.c_str());
        }
        break;
    }
}
        
void Engine::PBRPipeline::RemoveModel(const std::string& name)
{
    if (m_Models.find(name) != m_Models.end())
    {
        Model* model = m_Models[name];
        
        std::vector<Part>& allParts = model -> GetParts();
        for (auto& part : allParts)
        {
            for (size_t index = 0; index < part.objects.size(); index++)
            {
                std::string objectName = name + "_" + part.name + "_" + std::to_string(index);
                RemoveObject(objectName);
            }
        }
        
        model->BindShader(nullptr); 
        model->DisableLight();
            
        m_Models.erase(name);
    }
}
void Engine::PBRPipeline::RemoveObject(const std::string& name)
{
    if (m_Objects.find(name) != m_Objects.end())
    {
        Object* object = m_Objects[name];
        object->m_Material.shader = nullptr; 
        object->DisableLight();
        
        m_Objects.erase(name);
    }
    
    if (m_OpaqueObjects.find(name) != m_OpaqueObjects.end())
    {
        Object* object = m_OpaqueObjects[name];
        object->m_Material.shader = nullptr;
        object->DisableLight();
        
        m_OpaqueObjects.erase(name);
    }
    else if(m_TransparentObjects.find(name) == m_TransparentObjects.end())
    {
        Object* object = m_TransparentObjects[name];
        object->m_Material.shader = nullptr;
        object->DisableLight();
        
        m_TransparentObjects.erase(name);
    }
    else if (m_MaskedObjects.find(name) == m_MaskedObjects.end())
    {
        Object* object = m_MaskedObjects[name];
        object->m_Material.shader = nullptr;
        object->DisableLight();
        
        m_MaskedObjects.erase(name);
    }
}