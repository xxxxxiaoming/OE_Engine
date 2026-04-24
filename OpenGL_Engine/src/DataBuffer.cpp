#include <glad/glad.h>

#include "DataBuffer.h"
#include "Helper.h"

void Engine::DataBuffer::DeleteBuffer()
{
    if (m_BufferID != 0)
    {
        GLCALL(glDeleteBuffers(1, &m_BufferID)); 
        m_BufferID = 0;
    }
}
