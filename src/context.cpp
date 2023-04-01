#include "context.h"
#include "image.h"

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
        //    position            color
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,// top right, red
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,// bottom right, green
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,// bottom left, blue
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f// top left, yellow
    };
    // 인덱스 배열
    uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    // VAO 생성
    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW, verticles, sizeof(float) * 32);

    // 정점의 정보가 어떻게 생겼는지 알려주는 것
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 3);
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 6);

    // indices는 attribute array 생성할 필요 없음
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(uint32_t) * 6);

    ShaderPtr vertShader = Shader::CreateFromFile("./shader/texture.vs", GL_VERTEX_SHADER);
	ShaderPtr fragShader = Shader::CreateFromFile("./shader/texture.fs", GL_FRAGMENT_SHADER);
    if(!vertShader || !fragShader)
        return false;
	SPDLOG_INFO("vertex shader id: {}", vertShader->Get());
	SPDLOG_INFO("fragment shader id: {}", fragShader->Get());

    m_program = Program::Create({fragShader,vertShader});
    if(!m_program)
        return false;
	SPDLOG_INFO("program id: {}",m_program->Get());

    // // shader 코드 바깥쪽에서 shader를 설정해줄 수 있음
    // auto loc = glGetUniformLocation(m_program->Get(), "color");
    // m_program->Use();
    // glUniform4f(loc, 0.0f, 1.0f, 0.0f, 1.0f);

    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    auto image = Image::Load("./image/container.jpg");
    if(!image)
        return false;
    SPDLOG_INFO("image: {}x{}, {} channels",image->GetWidth(), image->GetHeight(), image->GetChannelCount());

    m_texture = Texture::CreateFromImage(image.get());
    
    auto image2 = Image::Load("./image/awesomeface.png");
    m_texture2 = Texture::CreateFromImage(image2.get());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture->Get());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->Get());

    m_program->Use();
    glUniform1i(glGetUniformLocation(m_program->Get(), "tex"), 0); // 0번 슬롯의 텍스쳐를 이용
    glUniform1i(glGetUniformLocation(m_program->Get(), "tex2"), 1); // 1번 슬롯의 텍스쳐를 이용

    // 0.5배 축소후 z축으로 90도 회전하는 행렬
    auto transform = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(2.f)),
                                glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto transformLoc = glGetUniformLocation(m_program->Get(), "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

    return true;
}

// Main의 while문에서 반복
void Context::Render(){
    glClear(GL_COLOR_BUFFER_BIT);
    m_program->Use();
    // primitive, count, type, pointer/offset
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}