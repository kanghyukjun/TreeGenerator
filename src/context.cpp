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
    glGenVertexArrays(1, &m_vertexArrayObject); // VAO의 갯수는 하나
    glBindVertexArray(m_vertexArrayObject); // 바인딩
    /*
    VBO(정점 데이터를 담은 버퍼 오브젝트), 색상 위치 텍스쳐 법선벡터 등의 정보 포함
    VBO 생성 및 정점 데이터를 GPU로 복사
    
    GL_STATIC_DRAW : vertex의 위치를 세팅 후 바꾸지 않을 것임
    GL_DYNAMIC_DRAW : vertex의 위치를 바꿀 것임
    GL_STREAM_DRAW : 버퍼를 생성 후 한번 그린 다음에 버릴 것임
    */
    glGenBuffers(1, &m_vertexBuffer); // 1개의 VBO 생성
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer); // 사용할 버퍼 지정
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, verticles, GL_STATIC_DRAW); // 데이터를 복사

    /*
    attribute의 생김새 지정
    정점의 n번째 attribute, point가 3개, 각각은 float 값,
    normalize 필요 없음, stride의 크기, 첫 정점의 해당 attribute 까지의 간격(offset)
    */
    glEnableVertexAttribArray(0); // 0번 attribute 사용할 것임
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float) * 3, 0);

    // indices는 attribute array 생성할 필요 없음
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);

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

    glUseProgram(m_program->Get());
    // primitive, count, type, pointer/offset
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}