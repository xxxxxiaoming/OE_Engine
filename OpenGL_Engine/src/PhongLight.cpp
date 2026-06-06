#include "PhongLight.h"

#include <filesystem>

#include "PLVS.h"
#include "PLFS.h"
#include "PLFSDR.h"
#include "PLVSDR.h"
#include "PLFS_SSAO.h"
#include "Random.h"

static const int MAX_NON_DIRECTIONAL_LIGHTS = 4;
static const int SSAO_SAMPLES_NUMBER = 64;
static const int SSAO_NOISE_NUMBER = 16;

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

void Engine::PhongLight::UnblockModelInternal(Model* model)
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

void Engine::PhongLight::UnblockObjectInternal(Object* object)
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

void Engine::PhongLight::GenerateShadowMapInternal(Renderer& renderer)
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
        
    m_ShadowMap.PostCapture(renderer);
    m_ShadowMap.SaveDepthMap("res/screenshot/depthmap.png");
}

void Engine::PhongLight::GenerateShadowMapPointInternal(Renderer& renderer)
{
    for (int index = 0; index < m_PointLightIndex; index++)
    {
        GenerateSpecificShadowMapPointInternal(index, renderer);
    }
}

void Engine::PhongLight::GenerateSpecificShadowMapPointInternal(int index, Renderer& renderer)
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
        
    // std::string savePath = "res/screenshot/depthCubeMap" + std::to_string(index) + std::string{'/'};
    shadowMapPoint.PostCapture(renderer);
    // shadowMapPoint.SaveDepthMap(savePath);
    
    m_PointLightsNeedCapture[index] = false;
    
    GLCALL(glBindTextureUnit(1 + index, shadowMapPoint.GetShadowMap()));
}

/*
 * @brief 管线中的前向渲染部分，只渲染透明物体，所有透明物体都会被放到这个流程中渲染
 */
void Engine::PhongLight::ForwardRenderInternal(const Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    m_RTLight.BindFramebuffer();
    m_ShaderLightForward.Use();
    
    renderer.EnableBlend();
        
    for (auto& objectPair : m_TransparentObjects)
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
    
    renderer.DisableBlend();
    
    m_ShaderLightForward.UnUse();
    m_RTLight.UnbindFramebuffer();
}

void Engine::PhongLight::DeferredRenderInternal(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
    
    renderer.EnableBlend();
    
    m_ShaderGBuffer.UnUse();
    m_RTGBuffer.UnbindFramebuffer();
    
    uint32_t positionBuffer = m_RTGBuffer.GetTextureBuffer(0);
    uint32_t lightSpacePosBuffer = m_RTGBuffer.GetTextureBuffer(1);
    uint32_t normalBuffer = m_RTGBuffer.GetTextureBuffer(2);
    uint32_t albedoSpecBuffer = m_RTGBuffer.GetTextureBuffer(3);
    
    GLCALL(glBindTextureUnit(6, positionBuffer));
    GLCALL(glBindTextureUnit(7, lightSpacePosBuffer));
    GLCALL(glBindTextureUnit(8, normalBuffer));
    GLCALL(glBindTextureUnit(9, albedoSpecBuffer));
    
    m_ShaderSSAO.Use();
    m_RTSSAO.BindFramebuffer();
    
    m_ShaderSSAO.SetUniform1i("u_PositionMap", 6);
    m_ShaderSSAO.SetUniform1i("u_NormalMap", 8);
    
    renderer.OnRender();
    
    m_RenderRect.OnDraw();
    
    renderer.DrawElements(m_RenderRect.GetIndexCount(), nullptr);
    
    m_ShaderSSAO.UnUse();
    m_RTSSAO.UnbindFramebuffer();
    
    m_ShaderSSAOSmoth.Use();
    m_RTSSAOSmoth.BindFramebuffer();
    
    uint32_t ssaoTexture = m_RTSSAO.GetTextureBuffer();
    
    GLCALL(glBindTextureUnit(10, ssaoTexture));
    m_ShaderSSAOSmoth.SetUniform1i("u_SSAOMap", 10);
    
    renderer.OnRender();
    
    m_RenderRect.OnDraw();
    
    renderer.DrawElements(m_RenderRect.GetIndexCount(), nullptr);
    
    
    m_ShaderSSAOSmoth.UnUse();
    m_RTSSAOSmoth.UnbindFramebuffer();
    
    uint32_t ssaoSmoothTexture = m_RTSSAOSmoth.GetTextureBuffer();
    GLCALL(glBindTextureUnit(10, ssaoSmoothTexture));
    
    // Generate final frame
    m_ShaderLight.Use();
    m_RTLight.BindFramebuffer();

    m_ShaderLight.SetUniform1i("u_PositionMap", 6);
    m_ShaderLight.SetUniform1i("u_DLightSpacePositionMap", 7);
    m_ShaderLight.SetUniform1i("u_NormalMap", 8);
    m_ShaderLight.SetUniform1i("u_AlbedoSpecMap", 9);
    m_ShaderLight.SetUniform1i("u_SSAOMap", 10);
    
    renderer.OnRender();
    
    m_RenderRect.OnDraw();
    
    renderer.DrawElements(m_RenderRect.GetIndexCount(), nullptr);
    
    m_ShaderLight.UnUse();
    m_RTLight.UnbindFramebuffer();
}

Engine::PhongLight::PhongLight(int shadowMapResolution, bool bDeffered) : 
    m_ShadowMap(shadowMapResolution, false),
    m_UseDeffered(bDeffered),
    m_ShadowMapResolution(shadowMapResolution),
    m_RTGBuffer(1280, 720, true),
    m_RTLight(1280, 720, true),
    m_RTSSAO(1280, 720, true),
    m_RTSSAOSmoth(1280, 720, true)
{
    m_ShadowMapPoint.reserve(MAX_NON_DIRECTIONAL_LIGHTS);
    m_PointLightsNeedCapture.reserve(MAX_NON_DIRECTIONAL_LIGHTS);
    
    for (int i = 0; i < MAX_NON_DIRECTIONAL_LIGHTS; i++)
        m_PointLightsNeedCapture.emplace_back(false);
    

    uint32_t colorAttachments[4] = {0,1,2,3};
        
    m_ShaderLightForward.CreateShaderFromSource(vsShaderCode, fsShaderCode);
    m_ShaderLight.CreateShaderFromSource(vsShaderCodeLight, fsShaderCodeLight);
    m_ShaderGBuffer.CreateShaderFromSource(vsShaderCodeDR, fsShaderCodeDR);
    m_ShaderSSAO.CreateShaderFromSource(vsShaderCodeLight, fsSSAOCode);
    m_ShaderSSAOSmoth.CreateShaderFromSource(vsShaderCodeLight, fsSSAOBlurCode);
    
    int resolution[2] = {1280, 720};
    std::vector<glm::vec3> samples;
    GenerateSSAOSamples(samples, m_SSAONoiseTexture);
    GLCALL(glBindTextureUnit(11, m_SSAONoiseTexture));
    m_ShaderSSAO.SetUniform3fv("u_Samples", samples.size(), samples.data());
    m_ShaderSSAO.SetUniform1iv("u_Resolution", 2, resolution);
    m_ShaderSSAO.SetUniform1i("u_NoiseMap", 11);
        
    m_RTGBuffer.CreateColorAttachment(0);
    m_RTGBuffer.CreateColorAttachment(1);
    m_RTGBuffer.CreateColorAttachment(2);
    m_RTGBuffer.CreateColorAttachment(3, GL_RGBA);
    m_RTGBuffer.CreateRenderBuffer();
    m_RTGBuffer.UseMultiColorAttachments(colorAttachments, 4);
    
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
	
    createRectangle(rectPos, widthHDRRect, heightHDRRect, vertices, indices);
    m_RenderRect(vertices, indices, 4, 6, assetDirector);
    m_RenderRect.DisableLight();
    
    
    /* 默认只开启方向光源 */
    m_ShaderLight.SetUniform1i("u_LightConfig.enableDirectionLight", 1);	// 开启方向光源
    m_ShaderLight.SetUniform1i("u_LightConfig.pointLightNum", 0);			// 不使用点光源
    m_ShaderLight.SetUniform1i("u_LightConfig.spotLightNum", 0);			// 不使用聚光源
    
    /* 方向光配置 */
    m_ShaderLight.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color.ambient", 0.5f, 0.5f, 0.5f);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color.diffuse", 1.0f, 1.0f, 1.0f);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color.specular", 0.7f, 0.7f, 0.7f);
    
     /* 默认只开启方向光源 */
    m_ShaderLightForward.SetUniform1i("u_LightConfig.enableDirectionLight", 1);	// 开启方向光源
    m_ShaderLightForward.SetUniform1i("u_LightConfig.pointLightNum", 0);			// 不使用点光源
    m_ShaderLightForward.SetUniform1i("u_LightConfig.spotLightNum", 0);			// 不使用聚光源
        
    /* 方向光配置 */
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color.ambient", 0.5f, 0.5f, 0.5f);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color.diffuse", 1.0f, 1.0f, 1.0f);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color.specular", 0.7f, 0.7f, 0.7f);
    
}

void Engine::PhongLight::TurnOn(Renderer& renderer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
    GLCALL(glBindTextureUnit(5, shadowMap));
    m_ShaderLight.SetUniform1i("u_ShadowDepthMap", 5);

    DeferredRenderInternal(renderer, viewMatrix, projectionMatrix);
    
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RTGBuffer.GetFBO()));
    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTLight.GetFBO()));
    GLCALL(glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_DEPTH_BUFFER_BIT, GL_NEAREST));
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    
    ForwardRenderInternal(renderer, viewMatrix, projectionMatrix);
}

void Engine::PhongLight::TurnOff(Renderer& renderer) const
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

/* Directional Light */

void Engine::PhongLight::EnableDirectionLight()
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform1i("u_LightConfig.enableDirectionLight", 1);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform1i("u_LightConfig.enableDirectionLight", 1);
}

void Engine::PhongLight::DisableDirectionLight()
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform1i("u_LightConfig.enableDirectionLight", 0);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform1i("u_LightConfig.enableDirectionLight", 0);
}

void Engine::PhongLight::ConfigDirectionLight(const vec3& direction, const vec3& ambient, const vec3& diffuse, const vec3& specular)
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform3f("u_DirectionLight.direction", direction.x, direction.y, direction.z);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLight.SetUniform3f("u_DirectionLight.color.specular", specular.x, specular.y, specular.z);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.direction", direction.x, direction.y, direction.z);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLightForward.SetUniform3f("u_DirectionLight.color.specular", specular.x, specular.y, specular.z);
}

/* Point Light */

bool Engine::PhongLight::AddPointLight(const vec3& position, const vec3& ambient, const vec3& diffuse, const vec3& specular, float constant, float linear, float quadratic)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || m_PointLightIndex >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(m_PointLightIndex) + "].";
    std::string uniformNamePointDepthMap = "u_PointLightsDepthMap[" + std::to_string(m_PointLightIndex) + "]";
    
    m_ShaderLight.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLight.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLight.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLight.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLight.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_ShaderLight.SetUniform1i(uniformNamePointDepthMap, m_PointLightIndex + 1);
    m_ShaderLight.SetUniform1i("u_LightConfig.pointLightNum", ++m_PointLightIndex);
    
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLightForward.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLightForward.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_ShaderLightForward.SetUniform1i("u_LightConfig.pointLightNum", m_PointLightIndex);
    
    m_ShadowMapPoint.emplace_back(m_ShadowMapResolution, false);
    m_PointLightsNeedCapture[m_PointLightIndex - 1] = true;
    
    return true;
}

bool Engine::PhongLight::ConfigPointLightPosition(int index, const vec3& position)
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

bool Engine::PhongLight::ConfigPointLightColor(int index, const vec3& ambient, const vec3& diffuse, const vec3& specular)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.specular", specular.x, diffuse.y, diffuse.z);
    
    m_ShaderLightForward.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.specular", specular.x, diffuse.y, diffuse.z);
    
    return true;
}

bool Engine::PhongLight::ConfigPointLightAttenuation(int index, float constant, float linear, float quadratic)
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

bool Engine::PhongLight::AddSpotLight(const vec3& position, const vec3& direction, const vec3& ambient, const vec3& diffuse, const vec3& specular, float innerAngle, float outterAngle, float constant, float linear, float quadratic)
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
    m_ShaderLight.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_ShaderLight.SetUniform1i("u_LightConfig.spotLightNum", ++m_SpotLightIndex);
    
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_ShaderLightForward.SetUniform1f(uniformName + "innerAngle", glm::radians(innerAngle));
    m_ShaderLightForward.SetUniform1f(uniformName + "outterAngle", glm::radians(outterAngle));
    m_ShaderLightForward.SetUniform1f(uniformName + "constant", constant);
    m_ShaderLightForward.SetUniform1f(uniformName + "linear", linear);
    m_ShaderLightForward.SetUniform1f(uniformName + "quadratic", quadratic);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_ShaderLightForward.SetUniform1i("u_LightConfig.spotLightNum", ++m_SpotLightIndex);
    
    return true;
}

bool Engine::PhongLight::ConfigSpotLightPosition(int index, const vec3& position)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    
    return true;
}

bool Engine::PhongLight::ConfigSpotLightDirection(int index, const vec3& direction)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    
    return true;
}


bool Engine::PhongLight::ConfigSpotLightColor(int index, const vec3& ambient, const vec3& diffuse, const vec3& specular)
{
    if (!m_ShaderLight.CheckShaderValidity() || !m_ShaderLightForward.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_ShaderLight.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLight.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    
    m_ShaderLightForward.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_ShaderLightForward.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    
    return true;
}

bool Engine::PhongLight::ConfigSpotLightAttenuation(int index, float constant, float linear, float quadratic)
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

bool Engine::PhongLight::ConfitSpotLightScale(int index, float innerAngle, float outterAngle)
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

void Engine::PhongLight::ConfigMVPMatrix(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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

void Engine::PhongLight::ConfigNormalMatrix(const glm::mat3& normalMatrix)
{
    if (!m_ShaderGBuffer.CheckShaderValidity())
        return;
        
    m_ShaderGBuffer.SetUniformMatrix3f("u_NormalMat", normalMatrix);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniformMatrix3f("u_NormalMat", normalMatrix);
    
}

void Engine::PhongLight::ConfigCameraWorldPosition(const glm::vec3& position)
{
    if (!m_ShaderLight.CheckShaderValidity())
        return;
    
    m_ShaderLight.SetUniform3f("u_CameraPosition", position.x, position.y, position.z);
    
    if (!m_ShaderLightForward.CheckShaderValidity())
        return;
    
    m_ShaderLightForward.SetUniform3f("u_CameraPosition", position.x, position.y, position.z);
}

void Engine::PhongLight::ConfigShadowMapCaptureView(glm::vec3 capturePosition, glm::vec3 captureLookAt, float viewLeft, float viewRight, float viewBottom, float viewTop, float viewNear, float viewFar)
{
    Engine::Camera shadowMapCaptureCam{capturePosition, captureLookAt, glm::vec3(0.0f, 1.0f, 0.0f)};
    glm::mat4 projectionMatrix = glm::ortho(viewLeft, viewRight, viewBottom, viewTop, viewNear, viewFar);
    glm::mat4 lightSpace = projectionMatrix * shadowMapCaptureCam.GetViewMatrix();
    
    m_ShaderGBuffer.SetUniformMatrix4f("u_LightSpace", lightSpace);
    
    m_ShadowMap.SetCaptureView(shadowMapCaptureCam, projectionMatrix);
}

void Engine::PhongLight::ConfigShadowMapPointCaptureView(int index, const glm::vec3& captureWorldPosition, float fov, float aspect, float nearPlane, float farPlane)
{
    m_ShaderLight.SetUniform1f("u_FarPlane", farPlane);
    m_ShadowMapPoint[index].SetCaptureView(captureWorldPosition, fov, aspect, nearPlane, farPlane);
    m_PointLightsNeedCapture[index] = true;
}

void Engine::PhongLight::AddModel(const std::string& name, Model* model)
{
    // model -> BindShader(m_UseDeffered ? &m_ShaderGBuffer : &m_ShaderLight); 
    // model -> EnableLight();
            
    if (m_Models.find(name) == m_Models.end())
        m_Models[name] = model;
    
    std::vector<Part>& allParts = model -> GetParts();
    for (auto& part : allParts)
    {   
        size_t index = 0;
        for (auto& object : part.objects)
        {
            std::string objectName = name + "_" + part.name + "_" + std::to_string(index);
            AddObject(objectName, &object);
            object.SetTransform(model->GetTransform());
        }
    }
}
void Engine::PhongLight::AddObject(const std::string& name, Object* object)
{
    if (m_Objects.find(name) == m_Objects.end())
        m_Objects[name] = object;
    
    switch (object->GetBlendMode())
    {
    case BlendMode::Opaque :
        if (m_OpaqueObjects.find(name) == m_OpaqueObjects.end())
        {
            object->m_Material.shader = &m_ShaderGBuffer;
            m_OpaqueObjects[name] = object;
        }
        break;
    case BlendMode::Masked :
        if (m_MaskedObjects.find(name) == m_MaskedObjects.end())
        {
            object->m_Material.shader = &m_ShaderGBuffer;
            m_MaskedObjects[name] = object;
        }
        break;
    case BlendMode::Transparent:
        if (m_TransparentObjects.find(name) == m_TransparentObjects.end())
        {
            object->m_Material.shader = &m_ShaderLightForward;
            m_TransparentObjects[name] = object;
        }
        break;
    }
}
        
void Engine::PhongLight::RemoveModel(const std::string& name)
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
void Engine::PhongLight::RemoveObject(const std::string& name)
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
