#include "context.h"
#include "image.h"
#include <imgui.h>
#include <cstring>
#include <regex>

ContextUPtr Context::Create(){
    auto context = ContextUPtr(new Context());
    if(!context->Init()){
        return nullptr;
    }
    return std::move(context);
}

void Context::ProcessInput(GLFWwindow* window) {
    const float cameraSpeed = 0.005f;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;    

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::MouseMove(double x, double y) {
    if(!m_cameraControl) return;

	auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    const float cameraRotSpeed = 0.08f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if(m_cameraYaw < 0.0f)   m_cameraYaw += 360.0f;
    if(m_cameraYaw > 360.0f) m_cameraYaw -= 360.0f;

    if(m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
    if(m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

    m_prevMousePos = pos;    
}

void Context::MouseButton(int button, int action, double x, double y) {
    if(button == GLFW_MOUSE_BUTTON_RIGHT) {
        if(action == GLFW_PRESS) {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if(action == GLFW_RELEASE) {
            m_cameraControl = false;
        }
    }
}

void Context::Reshape(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);

    m_framebuffer = Framebuffer::Create(Texture::Create(width, height, GL_RGBA));
}

std::string Context::MakeCodes() {
    std::string result = m_axiom;
    std::string tmp;
    for(int i=0; i<m_codesVector.size(); i++){
        tmp = m_codesVector.at(i);
        std::size_t pos = tmp.rfind('=');

        std::string condition = tmp.substr(0, pos);
        std::string replace = tmp.substr(pos + 1);
        
        for(int i=0; i<m_iteration; i++)
            result = std::regex_replace(result, std::regex(condition), replace);
    }

    // SPDLOG_INFO("string: {}", result);
    return result;
}

bool Context::Init(){
    glEnable(GL_MULTISAMPLE);
    m_box = Mesh::CreateBox();

    m_simpleProgram = Program::Create("./shader/simple.vs", "./shader/simple.fs");
    if(!m_simpleProgram) return false;

    m_program = Program::Create("./shader/lighting.vs", "./shader/lighting.fs");
    if(!m_program) return false;

    m_textureProgram = Program::Create("./shader/texture.vs", "./shader/texture.fs");
    if (!m_textureProgram) return false;

    m_postProgram = Program::Create("./shader/texture.vs", "./shader/gamma.fs");
    if (!m_postProgram) return false;

    m_lightingShadowProgram = Program::Create("./shader/lighting_shadow.vs", "./shader/lighting_shadow.fs");
    if (!m_lightingShadowProgram) return false;

    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    TexturePtr darkGrayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f)).get());

    TexturePtr grayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

    TexturePtr brownTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.6f, 0.4f, 0.2f, 1.0f)).get());

    TexturePtr greenTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.6f, 0.2f, 1.0f)).get());
            
    m_planeMaterial = Material::Create();
    m_planeMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/marble.jpg").get());
    m_planeMaterial->specular = grayTexture;
    m_planeMaterial->shininess = 4.0f;

    // m_box1Material = Material::Create();
    // m_box1Material->diffuse = Texture::CreateFromImage(Image::Load("./image/container.jpg").get());
    // m_box1Material->specular = darkGrayTexture;
    // m_box1Material->shininess = 16.0f;

    m_branchMaterial = Material::Create();
    m_branchMaterial->diffuse = brownTexture;
    m_branchMaterial->specular = brownTexture;
    m_branchMaterial->shininess = 4.0;

    m_leafMaterial = Material::Create();
    m_leafMaterial->diffuse = greenTexture;
    m_leafMaterial->specular = greenTexture;
    m_leafMaterial->shininess = 3.0f;

    m_plane = Mesh::CreatePlane();
    // m_windowTexture = Texture::CreateFromImage(Image::Load("./image/blending_transparent_window.png").get());

    // cube texture
    auto cubeRight = Image::Load("./image/skybox/right.jpg", false);
    auto cubeLeft = Image::Load("./image/skybox/left.jpg", false);
    auto cubeTop = Image::Load("./image/skybox/top.jpg", false);
    auto cubeBottom = Image::Load("./image/skybox/bottom.jpg", false);
    auto cubeFront = Image::Load("./image/skybox/front.jpg", false);
    auto cubeBack = Image::Load("./image/skybox/back.jpg", false);
    m_cubeTexture = CubeTexture::CreateFromImages({
        cubeRight.get(),
        cubeLeft.get(),
        cubeTop.get(),
        cubeBottom.get(),
        cubeFront.get(),
        cubeBack.get(),
    });
    m_skyboxProgram = Program::Create("./shader/skybox.vs", "./shader/skybox.fs");
    if(!m_skyboxProgram) return false;

    m_envMapProgram = Program::Create("./shader/env_map.vs", "./shader/env_map.fs");
    if(!m_envMapProgram) return false;

    m_shadowMap = ShadowMap::Create(1024,1024);

    return true;
}

// Main의 while문에서 반복
void Context::Render() {
    if(ImGui::Begin("UI window")) {
        if(ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor))){
            glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        // ImGui::DragFloat("gamma", &m_gamma, 0.01f, 0.0f, 2.0f);
        ImGui::Separator();
        ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.01f);
        ImGui::DragFloat("camera yaw",&m_cameraYaw, 0.05f);
        ImGui::DragFloat("camera pitch",&m_cameraPitch, 0.05f, -89.0f, 89.0f);
        ImGui::Separator();
        if(ImGui::Button("reset camera")){
            m_cameraPitch = -14.0f;
            m_cameraYaw = 0.0f;
            m_cameraPos = glm::vec3(0.0f, 4.0f, 12.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("l.directional", &m_light.directional);
            ImGui::DragFloat3("l.position", glm::value_ptr(m_light.position), 0.01f);
            ImGui::DragFloat3("l.direction", glm::value_ptr(m_light.direction), 0.01f);
            ImGui::DragFloat2("1.cutoff", glm::value_ptr(m_light.cutoff), 0.5f, 0.0f, 180.0f);
            ImGui::DragFloat("1.distance", &m_light.distance, 0.5f, 0.0f, 3000.0f);
            ImGui::ColorEdit3("l.ambient", glm::value_ptr(m_light.ambient));
            ImGui::ColorEdit3("l.diffuse", glm::value_ptr(m_light.diffuse));
            ImGui::ColorEdit3("l.specular", glm::value_ptr(m_light.specular));
            ImGui::Checkbox("1. blinn", &m_blinn);
        }
    }
    ImGui::End();

    if(ImGui::Begin("Tree")){
        ImGui::DragFloat("angle", &gui_angle, 0.1f, 20.0f, 70.0f);
        ImGui::DragFloat("radius", &gui_radius, 0.005f, 0.03f, 0.1f);
        ImGui::DragFloat("length", &gui_length, 0.03f, 0.3f, 2.0f);
        ImGui::DragInt("iteration", &m_iteration, 0.05f, 0, 4);
        ImGui::Separator();
        if(ImGui::Button("Draw")){
            m_newCodes = true;
            m_angle = gui_angle;
            m_cylinderRadius = gui_radius;
            m_cylinderHeight = gui_length;
        }
        if(ImGui::Button("Clear")){
            m_codes = "";
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("string", ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::TextWrapped("%s",m_codes.c_str());
        }
    }
    ImGui::End();

    // const char* treeType[] = {"binary tree", "arrow weed", "fuzzy weed", "twiggy weed", "tall seaweed", "wavy seaweed"};
    // const char* genType[] = {"context-free", "context-sensitive", "stochastic", "parametric"};
    // static int tree_current = 0;
    // static int gen_current = 0;
    // static char axiom[128] = "X";
    // static float angle = 10.0f;
    // static float iterations = 3;
    // static float length = 5.0f;
    // if(ImGui::Begin("Tree parameter")){
    //     ImGui::Combo("tree type", &tree_current, treeType, IM_ARRAYSIZE(treeType));
    //     ImGui::Combo("generate type", &gen_current, genType, IM_ARRAYSIZE(genType));
    //     ImGui::InputText("axiom", axiom, IM_ARRAYSIZE(axiom));
    //     ImGui::Separator();
    //     ImGui::DragFloat("angle", &angle, 0.1f, 10.0f, 85.0f);
    //     ImGui::DragFloat("iterations", &iterations, 1.0f, 1.0f, 6.0f);
    //     ImGui::DragFloat("length", &length, 0.1f, 1.0f, 10.0f);
    //     ImGui::Separator();
    //     if(ImGui::Button("draw"));
    //     ImGui::Separator();
    //     if(ImGui::CollapsingHeader("rules", ImGuiTreeNodeFlags_DefaultOpen)){
    //         static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    //         static char text[1024 * 4] = 
    //             "\n"
    //             "F -> FF\n\n"
    //             "X -> F-[[X]+X]+F[+FX]-X\n\n"
    //             "X -> F+[[X]-X]-F[-FX]+X";
    //         ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text),
    //         ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
    //     }
    // }
    // ImGui::End();

    // if(ImGui::Begin("view")){
    //     ImGui::Image((ImTextureID)m_shadowMap->GetShadowMap()->Get(),
    //         ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    // }
    // ImGui::End();

    auto lightView = glm::lookAt(m_light.position,
        m_light.position + m_light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto lightProjection = m_light.directional ?
        glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f) :
        glm::perspective(
            glm::radians((m_light.cutoff[0] + m_light.cutoff[1]) * 2.0f),
            1.0f, 1.0f, 20.0f);

    m_cylinder = Mesh::CreateCylinder(m_cylinderRadius, m_cylinderHeight);
    m_leaf = Mesh::CreateCylinder(m_leafRadius, m_leafHeight);

    if(m_newCodes){
        // 규칙에 의해 새로운 코드를 생성하는 코드
        m_codesVector.clear();
        m_codesVector.push_back("A=F[--&&&FC][++&&&FC][--^FC][++^FC]");
        m_codesVector.push_back("C=F[--<&&FC]||[++>&&FC]||[+<^^FC]||[->^^FC]");
        m_codes = MakeCodes();
        m_newCodes = false;
    }

    // shadowMap을 만들기 위해 shadowMap에 빛의 시점에서의 장면 그리기
    m_shadowMap->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0,
        m_shadowMap->GetShadowMap()->GetWidth(),
        m_shadowMap->GetShadowMap()->GetHeight());
    m_simpleProgram->Use();
    m_simpleProgram->SetUniform("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    DrawScene(lightProjection, lightView, m_simpleProgram.get()); // 빛의 위치에서 depth 값을 렌더링
    DrawTree(lightProjection, lightView, m_simpleProgram.get());

    Framebuffer::BindToDefault(); // 렌더링 종료, 원래 프로그램으로 복귀
    glViewport(0, 0, m_width, m_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);

    m_cameraFront =
        glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    // perspective
    auto projection = glm::perspective(glm::radians(45.0f), (float)m_width / (float)m_height, 0.1f, 100.0f);
    auto view = glm::lookAt(
        m_cameraPos,
        m_cameraPos + m_cameraFront,
        m_cameraUp);
    
    auto skyboxModelTransform =
        glm::translate(glm::mat4(1.0), m_cameraPos) * glm::scale(glm::mat4(1.0), glm::vec3(50.0f));
    m_skyboxProgram->Use();
    m_cubeTexture->Bind();
    m_skyboxProgram->SetUniform("skybox", 0);
    m_skyboxProgram->SetUniform("transform", projection * view * skyboxModelTransform);
    m_box->Draw(m_skyboxProgram.get());

    glm::vec3 lightPos = m_light.position;
    glm::vec3 lightDir = m_light.direction;

    // light 렌더링
    if(!m_light.directional){
        auto lightModelTransform = glm::translate(glm::mat4(1.0), m_light.position) *
            glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
        m_simpleProgram->Use();
        m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
        m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
        m_box->Draw(m_simpleProgram.get());
    }

    // camera & light
    m_lightingShadowProgram->Use();
    m_lightingShadowProgram->SetUniform("viewPos", m_cameraPos);
    m_lightingShadowProgram->SetUniform("light.directional", m_light.directional ? 1 : 0);
    m_lightingShadowProgram->SetUniform("light.position", m_light.position);
    m_lightingShadowProgram->SetUniform("light.direction", m_light.direction);
    m_lightingShadowProgram->SetUniform("light.cutoff", glm::vec2(
        cosf(glm::radians(m_light.cutoff[0])),
        cosf(glm::radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_lightingShadowProgram->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_lightingShadowProgram->SetUniform("light.ambient", m_light.ambient);
    m_lightingShadowProgram->SetUniform("light.diffuse", m_light.diffuse);
    m_lightingShadowProgram->SetUniform("light.specular", m_light.specular);
    m_lightingShadowProgram->SetUniform("blinn", (m_blinn ? 1 : 0));
    m_lightingShadowProgram->SetUniform("lightTransform", lightProjection * lightView);
    glActiveTexture(GL_TEXTURE3);
    m_shadowMap->GetShadowMap()->Bind();
    m_lightingShadowProgram->SetUniform("shadowMap", 3);
    glActiveTexture(GL_TEXTURE0);

    DrawScene(projection, view, m_lightingShadowProgram.get());
    if(!m_codes.empty())
        DrawTree(projection, view, m_lightingShadowProgram.get());
}

// 회전 후 이동 -> 이동행렬 * 회전행렬 (순서)
void Context::DrawTree(const glm::mat4& projection, const glm::mat4& view, const Program* program) {
    // const float angle = 30.0f;
    MatrixStack stack; // 행렬 연산을 위한 스택
    std::stack<int> stackCount; // pop 하는 수를 정하기 위한 스택
    std::stack<char> direction; // 나뭇잎의 방향을 정하기 위한 스택
    std::stack<int> directionCount; // 나뭇잎 방향 pop 하는 수 정하기 위한 스택

    double trigon_translate = sin(m_angle * M_PI / 180.0f) * (m_cylinderHeight/2.0f);

    // 회전행렬 1.1배 => 가중치
    stack.pushMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight/1.7f, 0.0f)));
    stackCount.push(0);
    directionCount.push(0);

    glm::mat4 rotateYMinus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * m_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotateYPlus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotateY180 = 
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 rotateXMinus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * m_angle), glm::vec3(1.0f, 0.0f, 0.0f)) * 
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.1f * trigon_translate));
    glm::mat4 rotateXPlus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.1f * trigon_translate));

    glm::mat4 rotateZMinus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * m_angle), glm::vec3(0.0f, 0.0f, 1.0f)) * 
        glm::translate(glm::mat4(1.0f), glm::vec3(1.1f * trigon_translate, 0.0f, 0.0f));
    glm::mat4 rotateZPlus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-1.1f * trigon_translate, 0.0f, 0.0f));

    glm::mat4 goFront = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight, 0.0f));

    int matrixTop = 0;
    int directionTop = 0;
    auto matrix_function = [&]()-> void {
        matrixTop = stackCount.top();
        matrixTop+=1;
        stackCount.pop();
        stackCount.push(matrixTop);

        directionTop = directionCount.top();
        directionTop+=1;
        directionCount.pop();
        directionCount.push(directionTop);
    };

    for(int i=0; i<m_codes.length(); i++){
        switch(m_codes.at(i)){
        case 'F': case 'X': case 'A': case 'C':
            matrixTop = stackCount.top();
            matrixTop+=1;
            stackCount.pop();
            stackCount.push(matrixTop);

            stack.pushMatrix(goFront); // 방향
            Context::DrawCylinder(projection, view, stack.getCurrentMatrix(), program);
            break;

        case '+':
            matrix_function();
            direction.push('+');
            stack.pushMatrix(rotateYPlus); // 방향
            break;

        case '-':
            matrix_function();
            direction.push('-');
            stack.pushMatrix(rotateYMinus); // 방향
            break;

        case '^':
            matrix_function();
            direction.push('^');
            stack.pushMatrix(rotateXPlus); // 방향
            break;

        case '&':
            matrix_function();
            direction.push('&');
            stack.pushMatrix(rotateXMinus); // 방향
            break;

        case '<':
            matrix_function();
            direction.push('<');
            stack.pushMatrix(rotateZPlus); // 방향
            break;

        case '>':
            matrix_function();
            direction.push('>');
            stack.pushMatrix(rotateZMinus); // 방향
            break;

        case '|':
            matrix_function();
            direction.push('|');
            stack.pushMatrix(rotateY180); // 방향
            break;

        case '[':
            stackCount.push(0);
            directionCount.push(0);
            break;

        case ']':
            if(m_codes.at(i-1) == 'X' || m_codes.at(i-1) == 'F' || m_codes.at(i-1) == 'A' || m_codes.at(i-1) == 'C') {
                // draw leaves
                Context::DrawLeaves(projection, view, stack.getCurrentMatrix(), program, direction.top());
            }
            for(int i=0; i<stackCount.top(); i++)
                stack.popMatrix();
            for(int i=0; i<directionCount.top(); i++)
                direction.pop();

            stackCount.pop();
            directionCount.pop();
            break;
        }
    }
}

void Context::DrawLeaves(const glm::mat4& projection, const glm::mat4& view,
    const glm::mat4 modelTransform, const Program* program, char direction){

    double trigon_translate = sin(m_angle * M_PI / 180.0f) * (m_leafHeight / 2.0f);
    double trigon_down = (-1.0f) * cos(m_angle * M_PI / 180.0f) * (2.0f * m_leafRadius);

    glm::mat4 rotateYMinus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * m_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotateYPlus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotateY180 = 
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 rotateXMinus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * m_angle), glm::vec3(1.0f, 0.0f, 0.0f)) * 
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, trigon_down, -1.0f * trigon_translate));
    glm::mat4 rotateXPlus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, trigon_down, 1.0f * trigon_translate));

    glm::mat4 rotateZMinus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * m_angle), glm::vec3(0.0f, 0.0f, 1.0f)) * 
        glm::translate(glm::mat4(1.0f), glm::vec3(1.0f * trigon_translate, trigon_down, 0.0f));
    glm::mat4 rotateZPlus = 
        glm::rotate(glm::mat4(1.0f), glm::radians(m_angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f * trigon_translate, trigon_down, 0.0f));


    glm::mat4 rotate;
    switch(direction){
    case '+':
        rotate = rotateYPlus;
        break;
    
    case '-':
        rotate = rotateYMinus;
        break;
    
    case '^':
        rotate = rotateXPlus;
        break;

    case '&':
        rotate = rotateXMinus;
        break;

    case '<':
        rotate = rotateZPlus;
        break;

    case '>':
        rotate = rotateZMinus;
        break;

    case '|':
        rotate = rotateY180;
        break;
    }

    program->Use();
    auto transform = projection * view * modelTransform * rotate;
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", glm::mat4(1.0f));
    m_leafMaterial->SetToProgram(program);
    m_leaf->Draw(program);
}

void Context::DrawCylinder(const glm::mat4& projection, const glm::mat4 view,
    const glm::mat4 modelTransform, const Program* program) {

    program->Use();
    auto transform = projection * view * modelTransform * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * m_cylinderHeight, 0.0f));
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", glm::mat4(1.0f));
    m_branchMaterial->SetToProgram(program);
    m_cylinder->Draw(program);
}

// context.cpp
void Context::DrawScene(const glm::mat4& projection, const glm::mat4& view, const Program* program) {
    // 바닥
    program->Use();
    auto modelTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    auto transform = projection * view * modelTransform;
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", modelTransform);
    m_planeMaterial->SetToProgram(program);
    m_box->Draw(program);

    // // 박스 1
    // modelTransform =
    //     glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.75f, -4.0f)) *
    //     glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
    //     glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    // transform = projection * view * modelTransform;
    // program->SetUniform("transform", transform);
    // program->SetUniform("modelTransform", modelTransform);
    // m_box1Material->SetToProgram(program);
    // m_box->Draw(program);
}