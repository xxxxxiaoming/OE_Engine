#include <glad/glad.h>

#include "IndexBuffer.h"
#include "Helper.h"

Engine::IndexBuffer::IndexBuffer()
{
    GLCALL(glGenBuffers(1, &m_BufferID));
}

Engine::IndexBuffer::~IndexBuffer()
{
    if (m_BufferID != 0)
    {
        GLCALL(glDeleteBuffers(1, &m_BufferID));
    }
}

void Engine::IndexBuffer::SetBufferData(GLsizeiptr size_t, const void* data, unsigned int usage) const
{
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_t, data, usage));
}

void Engine::IndexBuffer::Bind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID));
}

void Engine::IndexBuffer::Unbind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
