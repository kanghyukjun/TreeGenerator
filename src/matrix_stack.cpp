#include "matrix_stack.h"

// 4x4 identity 행렬로 초기화
MatrixStack::MatrixStack(){
    matrixStack.push(glm::mat4(1.0f));
}

bool MatrixStack::isEmpty() {
    if(matrixStack.empty()) return true;
    else return false;
}

// 가장 top에 있는 행렬에 matrix 적용하여 push
void MatrixStack::pushMatrix(const glm::mat4 matrix){
    matrixStack.push(matrixStack.top()*matrix);
}

// stack의 사이즈가 1이면 (identity 행렬밖에 없으면) 오류 발생
void MatrixStack::popMatrix(){
    if(matrixStack.size() == 1){
        SPDLOG_ERROR("failed to pop stack matrix");
    }
    else{
        matrixStack.pop();
    }
}

// stack이 비어있으면 (identity 행렬밖에 없으면) identity 행렬만 반환
glm::mat4 MatrixStack::getCurrentMatrix(){
    return matrixStack.top();
}