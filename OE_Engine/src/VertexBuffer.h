#pragma once
#include "DataBuffer.h"

namespace Engine
{
    class VertexBuffer : public DataBuffer
    {
    public:
        VertexBuffer();
        ~VertexBuffer() override;
        
        void SetBufferData(long long int size_t, const void* data, unsigned int usage) const override;
        void Bind() const override;
        void Unbind() const override;
    };
}
