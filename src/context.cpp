#include "context.h"

ContextUPtr Context::Create(){
    auto context = ContextUPtr(new Context());
    if(!context->Init()){
        return nullptr;
    }
    return std::move(context);
}

bool Context::Init(){
    // 정점의 데이터
    float verticles[] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f 
    };
    // 인덱스 배열
    uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    // VAO 생성
    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW, verticles, sizeof(float) * 12);

    m_vertexLayout->SetAttrib(0,3,GL_FLOAT,GL_FALSE,sizeof(float) * 3, 0);

    // indices는 attribute array 생성할 필요 없음
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(uint32_t) * 6);

    ShaderPtr vertShader = Shader::CreateFromFile("./shader/simple.vs", GL_VERTEX_SHADER);
	ShaderPtr fragShader = Shader::CreateFromFile("./shader/simple.fs", GL_FRAGMENT_SHADER);
    if(!vertShader || !fragShader)
        return false;
	SPDLOG_INFO("vertex shader id: {}", vertShader->Get());
	SPDLOG_INFO("fragment shader id: {}", fragShader->Get());

    m_program = Program::Create({fragShader,vertShader});
    if(!m_program)
        return false;
	SPDLOG_INFO("program id: {}",m_program->Get());

    glClearColor(0.0f, 0.1f, 0.2f, 0.3f);

    return true;
}

void Context::Render(){
    glClear(GL_COLOR_BUFFER_BIT);

    m_program->Use();
    // primitive, count, type, pointer/offset
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}