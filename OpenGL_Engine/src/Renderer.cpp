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

void Engine::Renderer::DrawElements(int count, const unsigned int* indices) const
{
    GLCALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, indices));
}

void Engine::Renderer::OnRender() const
{
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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

// void Engine::Renderer::AddDataBuffer(const unsigned int bufferID)
// {
//     m_DataBufferArray.push_back(bufferID);
// }

// void Engine::Renderer::AddVAOBuffer(const unsigned int bufferID)
// {
//     m_VaoArray.push_back(bufferID);
// }
