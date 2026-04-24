#include <glad/glad.h>
#include "VertexArrayBuffer.h"
#include "Helper.h"

Engine::VertexArrayBuffer::VertexArrayBuffer()
{
    GLCALL(glGenVertexArrays(1, &m_VAO));
}

Engine::VertexArrayBuffer::~VertexArrayBuffer()
{
    if (m_VAO == 0) return;
    GLCALL(glDeleteVertexArrays(1, &m_VAO));
}

void Engine::VertexArrayBuffer::Bind() const
{
    GLCALL(glBindVertexArray(m_VAO));
}

void Engine::VertexArrayBuffer::Unbind() const
{
    GLCALL(glBindVertexArray(0));
}

void Engine::VertexArrayBuffer::Delete()
{
    if (m_VAO != 0)
    {
        GLCALL(glDeleteVertexArrays(1, &m_VAO));
        m_VAO = 0;
    }
}
