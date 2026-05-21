#include "ShadowMap.h"

/* language=GLSL */
constexpr char vs[] = R"glsl(
#version 460 core

layout (location=0) in vec3 VertexPosition;

uniform mat4 u_LightSpace;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightSpace * u_Model * vec4(VertexPosition, 1.0);
}

)glsl";

/* language=GLSL */
constexpr char fs[] = R"glsl(
#version 460 core

void main()
{
    // Nothing to do! Yes!
}
)glsl";

Engine::ShadowMap::ShadowMap(int resolution, bool debug) : 
    m_RenderTarget(resolution, resolution), 
    m_Shader(vs, fs), 
    m_Resolution(resolution),
    m_Debug(debug)
{
    if (m_Debug)
    {
        m_RenderTarget.CreateColorAttachment();
        m_RenderTarget.CreateRenderBuffer();
    }
    else
    {
        m_RenderTarget.BindFramebuffer();
        m_RenderTarget.SetDepthUseOnly();
        m_RenderTarget.CreateDepthAttachment();  
    }
}


void Engine::ShadowMap::OnCapture(const Renderer& renderer) const
{
    m_RenderTarget.BindFramebuffer();
    GLCALL(glViewport(0, 0, m_Resolution, m_Resolution));
    renderer.OnRender();
}

void Engine::ShadowMap::CaptureModel(const glm::mat4& modelMatrix, Model& model, Engine::Renderer& renderer)
{
    m_Shader.Use();
    m_Shader.SetUniformMatrix4f("u_Model", modelMatrix);
    model.Draw(renderer);
}

void Engine::ShadowMap::CaptureObject(const glm::mat4& modelMatrix, Object& object, const Renderer& renderer)
{
    m_Shader.Use();
    m_Shader.SetUniformMatrix4f("u_Model", modelMatrix);
    object.OnDraw();
    renderer.DrawElements(object.GetIndexCount(), nullptr);
}

void Engine::ShadowMap::PostCapture(Renderer& renderer) const
{
    m_RenderTarget.UnbindFramebuffer();
    m_Shader.UnUse();
    
    uint32_t viewPortX, viewPortY;
    int width, height;
    
    renderer.GetViewPortAll(viewPortX, viewPortY, width, height);
    GLCALL(glViewport(viewPortX, viewPortY, width, height));
}

void Engine::ShadowMap::SetCaptureView(const Camera& captureCamera, const glm::mat4& projectionMatrix)
{
    glm::mat4 captureView = captureCamera.GetViewMatrix();
    glm::mat4 captureSpace = projectionMatrix * captureView;
    m_Shader.Use();
    m_Shader.SetUniformMatrix4f("u_LightSpace", captureSpace);
    m_Shader.UnUse();
}

void Engine::ShadowMap::SaveDepthMap(const std::string& path) const
{
    if (m_Debug)
    {
        m_RenderTarget.SaveColorAttachment(path);
    }
    else
    {
        m_RenderTarget.SaveDepthAttachment(path);
    }
}
