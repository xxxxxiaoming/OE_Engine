#pragma once
#include <GLFW/glfw3.h>

namespace  Engine
{
    class Renderer
    {
    private:
        GLFWwindow* m_Window = nullptr;
        unsigned int m_ViewportX = 0;
        unsigned int m_ViewportY = 0;
        int m_ViewportWidth = 0;
        int m_ViewportHeight = 0;
        
        // std::vector<unsigned int> m_DataBufferArray; /* VBO + IBO */
        // std::vector<unsigned int> m_VaoArray;         /* VAO only */
    public:
        Renderer(int width, int height, const char* title);
        // ~Renderer();
    
        void SetViewportSize(int width, int height);
        void SetViewportPosition(unsigned int x, unsigned int y);
        void SetViewportAll(unsigned int x, unsigned int y, int width, int height);
        
        const GLFWwindow* GetGLFWwinow() const;
        bool CheckWindowShouldClose() const { return glfwWindowShouldClose(const_cast<GLFWwindow*>(m_Window)); }
        
        void EnableBlend() const;
        void DisableBlend() const;
        
        void EnableDepthTest() const;
        void DisableDepthTest() const;
        
        void DrawElements(int count, const unsigned int* indices) const;
        void OnRender() const;
        void Render() const;
        void Clear() const {glfwTerminate();}
        
        /* 加入到 dataBufferArray 的 data buffer 在运行结束时无需手动 delete buffer */
        // void AddDataBuffer(const unsigned int bufferID);
        
        /* 加入到 vaoArray 的 vao 在运行结束时无需手动 delete buffer */
        // void AddVAOBuffer(const unsigned int bufferID);
    };   
}
