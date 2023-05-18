#ifndef __MATRIX_STACK_H__
#define __MATRIX_STACK_H__

#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "common.h"

class MatrixStack {
public:
    MatrixStack(float x, float z);
    MatrixStack();
    void pushMatrix(const glm::mat4 matrix);
    void popMatrix();
    bool isEmpty();
    glm::mat4 getCurrentMatrix();

private:
    std::stack<glm::mat4> matrixStack;
};

#endif // __MATRIX_STACK_H__