#include <glad/glad.h>
#include <iostream>

#include "Renderer.h"
#include "Helper.h"

Engine::Renderer::Renderer(int width, int height, const char* title)
{
    if (!glfwInit())
        return;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Open core profile.

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    
    if (m_Window == nullptr)
    {
        glfwTerminate();
        return;
    }
    
    glfwMakeContextCurrent(m_Window);
    
    int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    if (!version)
    {
        glfwTerminate();
        return;
    }
    else
    {
        std::cout << glGetString(GL_VERSION) << '\n';
    }
    
    //int glfwFrameBufferSizeW, glfwFrameBufferSizeH;
    //glfwGetFramebufferSize(m_Window, &glfwFrameBufferSizeW, &glfwFrameBufferSizeH);
    
    m_ViewportX = 0;
    m_ViewportY = 0;
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    GLCALL(glViewport(0, 0, width, height));
}

// Engine::Renderer::~Renderer()
// {
//     GLCALL(glDeleteBuffers(static_cast<GLsizei>(m_DataBufferArray.size()), m_DataBufferArray.data()));
//     GLCALL(glDeleteVertexArrays(static_cast<GLsizei>(m_VaoArray.size()), m_VaoArray.data()));
//     GLCALL(glfwTerminate());
// }

void Engine::Renderer::SetViewportSize(int width, int height)
{
    GLCALL(glViewport(m_ViewportX, m_ViewportY, width, height));
    
    m_ViewportWidth = width;
    m_ViewportHeight = height;
}

void Engine::Renderer::SetViewportPosition(unsigned int x, unsigned int y)
{
    GLCALL(glViewport(x, y, m_ViewportWidth, m_ViewportHeight));
    
    m_ViewportX = x;
    m_ViewportY = y;
}

void Engine::Renderer::SetViewportAll(unsigned int x, unsigned int y, int width, int height)
{
    GLCALL(glViewport(x, y, width, height));
    
    m_ViewportX = x;
    m_ViewportY = y;
    m_ViewportWidth = width;
    m_ViewportHeight = height;
}

const GLFWwindow* Engine::Renderer::GetGLFWwinow() const
{
    return m_Window;
}

void Engine::Renderer::DrawElements(int count, const unsigned int* indices, uint32_t mode) const
{
    GLCALL(glDrawElements(mode, count, GL_UNSIGNED_INT, indices));
}

void Engine::Renderer::DrawElementsInstanced(int count, const unsigned int* indices, uint32_t amount, uint32_t mode) const
{
    GLCALL(glDrawElementsInstanced(mode, count, GL_UNSIGNED_INT, indices, amount));
}

void Engine::Renderer::OnRender() const
{
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void Engine::Renderer::Render() const
{    
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

void Engine::Renderer::EnableBlend() const
{
    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void Engine::Renderer::DisableBlend() const
{
    GLCALL(glDisable(GL_BLEND));
}

void Engine::Renderer::EnableDepthTest() const
{
    GLCALL(glEnable(GL_DEPTH_TEST));
}

void Engine::Renderer::DisableDepthTest() const
{
    GLCALL(glDisable(GL_DEPTH_TEST));
}

void Engine::Renderer::EnebleStencilTest() const
{
    GLCALL(glEnable(GL_STENCIL_TEST));
}

void Engine::Renderer::DisableStencilTest() const
{
    GLCALL(glDisable(GL_STENCIL_TEST));
}

void Engine::Renderer::SetStencilFunc(uint32_t func, int ref, uint32_t mask) const
{
    GLCALL(glStencilFunc(func, ref, mask));
}

void Engine::Renderer::SetStencilOp(uint32_t sfail, uint32_t dfail, uint32_t dpass) const
{
    GLCALL(glStencilOp(sfail, dfail, dpass));
}

// void Engine::Renderer::AddDataBuffer(const unsigned int bufferID)
// {
//     m_DataBufferArray.push_back(bufferID);
// }

// void Engine::Renderer::AddVAOBuffer(const unsigned int bufferID)
// {
//     m_VaoArray.push_back(bufferID);
// }
