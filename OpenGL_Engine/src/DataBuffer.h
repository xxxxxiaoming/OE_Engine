#pragma once

namespace Engine
{
    class DataBuffer
    {
    protected:
        unsigned int m_BufferID = 0;
    public:
        DataBuffer() = default;
        virtual ~DataBuffer() = default;
        
        unsigned int GetBufferID() const { return m_BufferID; }
        void DeleteBuffer();
        
        virtual void SetBufferData(long long int size_t, const void* data, unsigned int usage) const = 0;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
    };
}