#include <glad/glad.h>
#include "VertexBuffer.h"
#include "Helper.h"

Engine::VertexBuffer::VertexBuffer()
{
    GLCALL(glGenBuffers(1, &m_BufferID));
}

Engine::VertexBuffer::~VertexBuffer()
{
    if (m_BufferID != 0)
    {
        GLCALL(glDeleteBuffers(1, &m_BufferID));
    }
        
}

void Engine::VertexBuffer::SetBufferData(GLsizeiptr size_t, const void* data, unsigned int usage) const
{
    GLCALL(glBufferData(GL_ARRAY_BUFFER, size_t, data, usage));
}

void Engine::VertexBuffer::Bind() const
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_BufferID));
}

void Engine::VertexBuffer::Unbind() const 
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
