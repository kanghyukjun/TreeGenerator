#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"

CLASS_PTR(Context)
class Context{
public:
    static ContextUPtr Create();
    void Render();

private:
    Context(){}
    bool Init();
    ProgramUPtr m_program;

    uint32_t m_vertexArrayObject; // VAO
    uint32_t m_vertexBuffer; // VBO
    uint32_t m_indexBuffer;
};

#endif // __CONTEXT_H__