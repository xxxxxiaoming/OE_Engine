#pragma once
#include "DataBuffer.h"

namespace Engine
{
    class IndexBuffer : public DataBuffer
    {
    public:
        IndexBuffer();
        ~IndexBuffer() override;
        
        void SetBufferData(long long int size_t, const void* data, unsigned int usage) const override;
        void Bind() const override;
        void Unbind() const override;
    };
}