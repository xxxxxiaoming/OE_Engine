#pragma once

namespace Engine
{
    class VertexAtrribArray
    {
    public:
        VertexAtrribArray() = delete;
        
        static void SetPointer(unsigned int index, int size, int type, int normalized, int stride, int pointer);
        
        static void Enable(unsigned int index);
        static void Disable(unsigned int index);
    };
}
