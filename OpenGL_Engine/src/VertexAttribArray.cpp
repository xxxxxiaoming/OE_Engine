#include <glad/glad.h>
#include "VertexAttribArray.h"
#include "Helper.h"


void Engine::VertexAttribArray::SetPointer(unsigned int index, int size, int type ,int normalized, int stride, int pointer)
{
    /* 参数列表
    ** void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
    ** index 属性的索引
    ** size number of components，属性的维数，比如这个三角形，每个顶点都是二维的，这个参数就传2
    ** type 属性的数据类型
    ** normalized 是否将属性值映射到[0, 1]这个区间中
    ** stride 两个属性之间的偏移值
    ** pointer 当前属性第一个值在buffer中的偏移值
    ** 比如
    ** [  位置xyz  |  颜色rgb  |  UV uv  ]
    ** [  12字节   |  12字节   |  8字节  ]
    ** rgb 的 stride 就是 32, 第一个rgb的pointer = 12，注意，这里不是 *pointer = 12！！！
    */
    if (type == GL_INT)
        GLCALL(glVertexAttribIPointer(index, size, GL_INT, stride, (const void*)pointer));
    else 
        GLCALL(glVertexAttribPointer(index, size, type, normalized, stride, (const void*)pointer));
}

void Engine::VertexAttribArray::Enable(unsigned int index)
{
    GLCALL(glEnableVertexAttribArray(index));    
}

void Engine::VertexAttribArray::Disable(unsigned int index)
{
    GLCALL(glDisableVertexAttribArray(index));
}

