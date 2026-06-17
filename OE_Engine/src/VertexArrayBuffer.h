#pragma once

namespace Engine
{
    class VertexArrayBuffer
    {
    private:
        unsigned int m_VAO = 0;
    public:
        VertexArrayBuffer();
        ~VertexArrayBuffer();
    
        void Delete();
        unsigned int GetVAOID() const { return m_VAO; }
        void Bind() const;
        void Unbind() const;
    };
}
