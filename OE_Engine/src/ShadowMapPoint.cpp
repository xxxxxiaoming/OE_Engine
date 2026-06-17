#include "ShadowMapPoint.h"

/* language=GLSL */
const char vsSource[] = R"glsl(
#version 460 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 u_Model;

void main()
{
    gl_Position = u_Model * vec4(VertexPosition, 1.0);
}

)glsl";

/* language=GLSL */
const char gsSource[] = R"glsl(
#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out; // max_vertices must be 18! Not 3!!!

out vec3 v_FragPosition;

uniform mat4 u_LightSpaceMatrices[6];

void main()
{
    for(int i = 0; i < 6; i++)
    {
        gl_Layer = i;
        for(int j = 0; j < 3; j++)
        {
            gl_Position = u_LightSpaceMatrices[i] * gl_in[j].gl_Position;
            v_FragPosition = gl_in[j].gl_Position.xyz;
            EmitVertex();
        }

        EndPrimitive();
    }
}

)glsl";

/* language=GLSL */
const char fsSource[] = R"glsl(
#version 460 core

in vec3 v_FragPosition;

uniform vec3 u_LightWorldPosition;
uniform float u_FarPlane;

void main()
{
    gl_FragDepth = length(u_LightWorldPosition - v_FragPosition) / u_FarPlane;
}

)glsl";

Engine::ShadowMapPoint::ShadowMapPoint() :
    m_RenderTarget(1024, false),
    m_Shader(vsSource, fsSource)
{
    
}

Engine::ShadowMapPoint::ShadowMapPoint(int resolution, bool debug) :
    m_RenderTarget(resolution, resolution), m_Shader(vsSource, fsSource, gsSource), m_Resolution(resolution), m_Debug(debug)
{
    if(m_Debug)
    {
        m_RenderTarget.CreateColorAttachment();
        m_RenderTarget.CreateRenderBuffer();
    }
    else
    {
        m_RenderTarget.CreateDepthCubeAttachment();
        m_RenderTarget.SetDepthUseOnly();    
    }
}

void Engine::ShadowMapPoint::OnCapture(Renderer& renderer) const
{
    m_RenderTarget.BindFramebuffer();
    m_Shader.Use();
    GLCALL(glViewport(0, 0, m_Resolution, m_Resolution));
    renderer.OnRender();
}

void Engine::ShadowMapPoint::PostCapture(Renderer& renderer) const
{
    m_RenderTarget.UnbindFramebuffer();
    m_Shader.UnUse();
    
    uint32_t viewportX, viewportY;
    int viewPortW, viewPortH;
    
    renderer.GetViewPortAll(viewportX, viewportY, viewPortW, viewPortH);
    GLCALL(glViewport(viewportX, viewportY, viewPortW, viewPortH));
}

void Engine::ShadowMapPoint::CaptureModel(const glm::mat4& modelMatrix, Model& model, Renderer& renderer)
{
    m_Shader.SetUniformMatrix4f("u_Model", modelMatrix);
    model.Draw(renderer);
}

void Engine::ShadowMapPoint::CaptureObject(const glm::mat4& modelMatrix, Object& object, Renderer& renderer)
{
    m_Shader.SetUniformMatrix4f("u_Model", modelMatrix);
    object.OnDraw();
    renderer.DrawElements(object.GetIndexCount(), nullptr);
}

void Engine::ShadowMapPoint::SetCaptureView(const glm::vec3& captureWorldPosition, float fov, float aspect, float nearPlane, float farPlane)
{
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    glm::vec3 lookAtDirection[6] = {
        glm::vec3{1.0f, 0.0f, 0.0f},  // positive x
        glm::vec3{-1.0f, 0.0f, 0.0f}, // negative x
        glm::vec3{0.0f, 1.0f, 0.0f},  // positive y
        glm::vec3{0.0f, -1.0f, 0.0f}, // negative y
        glm::vec3{0.0f, 0.0f, 1.0f},  // positive z
        glm::vec3{0.0f, 0.0f, -1.0f}, // negative z
        
    };
    glm::vec3 upDirection[6] = {
        glm::vec3{0.0f, -1.0f, 0.0f},  // positive x
        glm::vec3{0.0f, -1.0f, 0.0f},  // negative x
        glm::vec3{0.0f, 0.0f, 1.0f},  // positive y
        glm::vec3{0.0f, 0.0f, -1.0f}, // negative y
        glm::vec3{0.0f, -1.0f, 0.0f},  // positive z
        glm::vec3{0.0f, -1.0f, 0.0f},  // negative z
    };
    glm::mat4 lightSpaceMatrices[6] = {};
    
    for (int i = 0; i < 6; i++)
    {
        glm::mat4 viewMatrix = glm::lookAt(captureWorldPosition, lookAtDirection[i] + captureWorldPosition, upDirection[i]);
        lightSpaceMatrices[i] =  projectionMatrix * viewMatrix;
    }
    
    m_Shader.SetUniformMatrix4fv("u_LightSpaceMatrices", 6, lightSpaceMatrices);
    m_Shader.SetUniform3f("u_LightWorldPosition", captureWorldPosition.x, captureWorldPosition.y, captureWorldPosition.z);
    m_Shader.SetUniform1f("u_FarPlane", farPlane);
}

void Engine::ShadowMapPoint::UpdateCapturePosition(const glm::vec3 captureWorldPosition)
{
    m_Shader.SetUniform3f("u_LightWorldPosition", captureWorldPosition.x, captureWorldPosition.y, captureWorldPosition.z);
}
