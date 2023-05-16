#include "context.h"
#include "image.h"
#include "glm/gtx/string_cast.hpp"
#include <regex>
#include <thread>
#include <cstdlib>
#include <random>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tuple>

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
    m_width = std::max(1, width);
    m_height = std::max(1, height);
    glViewport(0, 0, m_width, m_height);

    m_framebuffer = Framebuffer::Create(Texture::Create(width, height, GL_RGBA));
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

    m_cylinderTexture = Texture::CreateFromImage(Image::Load("./image/wood.png").get());
    m_cylinderProgram = Program::Create("./shader/cylinder.vs", "./shader/cylinder.fs");
    if(!m_cylinderProgram) return false;

    m_leafTexture = Texture::CreateFromImage(Image::Load("./image/leaf2.png").get());
    m_leafProgram = Program::Create("./shader/leaf.vs", "./shader/leaf.fs");
    if(!m_leafProgram) return false;

    m_objProgram = Program::Create("./shader/obj.vs", "./shader/obj.fs");
    if(!m_objProgram) return false;

    m_treeTexture = Texture::CreateFromImage(Image::Load("./image/tree.png").get());

    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    TexturePtr darkGrayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f)).get());

    TexturePtr grayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

    m_brownTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.6f, 0.4f, 0.2f, 1.0f)).get());

    TexturePtr greenTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.6f, 0.2f, 1.0f)).get());


    m_planeMaterial = Material::Create();
    m_planeMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/marble.jpg").get());
    m_planeMaterial->specular = grayTexture;
    m_planeMaterial->shininess = 4.0f;

    m_branchMaterial = Material::Create();
    m_branchMaterial->diffuse = m_brownTexture;
    m_branchMaterial->specular = m_brownTexture;
    m_branchMaterial->shininess = 4.0;

    m_leafMaterial = Material::Create();
    m_leafMaterial->diffuse = greenTexture;
    m_leafMaterial->specular = greenTexture;
    m_leafMaterial->shininess = 3.0f;

    m_plane = Mesh::CreatePlane();

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
    if (ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Open", "Ctrl+O")) {
                m_fileDialogOpen.SetTitle("Select *.obj");
                m_fileDialogOpen.SetTypeFilters({ ".obj"});
                m_fileDialogOpen.Open();
            }
            if(ImGui::MenuItem("Save", "Ctrl+S")) {
                m_fileDialogSave.SetTitle("Select Folder");
                m_fileDialogSave.SetTypeFilters({".obj"});
                m_fileDialogSave.Open();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // 만약 사용자가 Open 메뉴를 선택했다면
    m_fileDialogOpen.Display();
    if(m_fileDialogOpen.HasSelected()) {
        OpenObject(m_fileDialogOpen);
    }
    m_fileDialogOpen.ClearSelected();

    // 만약 사용자가 Save 메뉴를 선택했다면
    m_fileDialogSave.Display();
    if(m_fileDialogSave.HasSelected()) {
        SaveObject(m_fileDialogSave);
    }
    m_fileDialogSave.ClearSelected();

    if (ImGui::Begin("UI window")) {
        ImGui::BeginChild("child1", ImVec2(0, 0), true);
        if(ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor))) {
            glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        ImGui::Separator();
        ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.01f);
        ImGui::DragFloat("camera yaw",&m_cameraYaw, 0.05f);
        ImGui::DragFloat("camera pitch",&m_cameraPitch, 0.05f, -89.0f, 89.0f);
        ImGui::Separator();
        if(ImGui::Button("reset camera")) {
            m_cameraPitch = -14.0f;
            m_cameraYaw = 0.0f;
            m_cameraPos = glm::vec3(0.0f, 4.0f, 12.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("directional", &m_light.directional);
            ImGui::DragFloat3("position", glm::value_ptr(m_light.position), 0.01f);
            ImGui::DragFloat3("direction", glm::value_ptr(m_light.direction), 0.01f);
            ImGui::DragFloat2("cutoff", glm::value_ptr(m_light.cutoff), 0.5f, 0.0f, 180.0f);
            ImGui::DragFloat("distance", &m_light.distance, 0.5f, 0.0f, 3000.0f);
            ImGui::ColorEdit3("ambient", glm::value_ptr(m_light.ambient));
            ImGui::ColorEdit3("diffuse", glm::value_ptr(m_light.diffuse));
            ImGui::ColorEdit3("specular", glm::value_ptr(m_light.specular));
            ImGui::Checkbox("blinn", &m_blinn);
            ImGui::Checkbox("floor", &m_floor);
            ImGui::Checkbox("scenery", &m_scenery);
        }
        ImGui::EndChild();
        ImGui::End();
    }

    if(ImGui::Begin("Tree")) {
        ImGui::BeginChild("child2", ImVec2(0, 0), true);
        ImGui::Text("tree");
        ImGui::DragFloat("angle", &m_gui_angle, 0.1f, 20.0f, 30.0f);
        ImGui::DragFloat("tree radius", &m_gui_radius, 0.005f, 0.05f, 0.3f);
        ImGui::DragFloat("tree length", &m_gui_length, 0.03f, 0.3f, 2.0f);
        ImGui::Separator();
        ImGui::Text("leaf");
        ImGui::DragFloat("leaf radius", &m_gui_leaf_radius, 0.005f, 0.1f, 0.3f);
        ImGui::DragFloat("leaf length", &m_gui_leaf_height, 0.005f, 0.1f, 0.3f);
        ImGui::Separator();
        ImGui::InputText("axiom", m_gui_axiom, sizeof(m_gui_axiom));
        ImGui::Combo("rules", &m_currentItem, m_comboItems, NUM_RULES);
        ImGui::InputTextMultiline("##rules", m_gui_rules, IM_ARRAYSIZE(m_gui_rules),
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5), ImGuiInputTextFlags_AllowTabInput);
        ImGui::Separator();
        ImGui::DragInt("iteration", &m_iteration, 0.05f, 0, 4);
        if(ImGui::Button("Draw")) {
            m_model.reset();
            // m_floor = true;
            m_newCodes = true;
            m_angle = m_gui_angle;
            m_cylinderRadius = m_gui_radius;
            m_cylinderHeight = m_gui_length;
            m_leafRadius = m_gui_leaf_radius;
            m_leafHeight = m_gui_leaf_height;
        }
        ImGui::SameLine();
        if(ImGui::Button("Clear")) {
            Clear();
            m_model.reset();
            // m_floor = true;
        }
        ImGui::BeginChild("child3", ImVec2(0, 0), true);
        if (ImGui::CollapsingHeader("string", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("%s",m_codes.c_str());
        }
        ImGui::EndChild();
        ImGui::EndChild();
        ImGui::End();
    }

    if(m_currentItem != CUSTOM_RULES)
        SetRules();

    auto lightView = glm::lookAt(m_light.position,
        m_light.position + m_light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto lightProjection = m_light.directional ?
        glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f) :
        glm::perspective(glm::radians((m_light.cutoff[0] + m_light.cutoff[1]) * 2.0f), 1.0f, 1.0f, 20.0f);

    m_cylinder = Mesh::CreateCylinder(m_cylinderRadius, m_cylinderHeight, m_radiusScaling);
    m_leaf = Mesh::CreateLeaf(m_leafRadius, m_leafHeight);

    if(m_newCodes){
        // 규칙에 의해 새로운 코드를 생성하는 코드
        m_codesVector.clear();

        std::istringstream ss(m_gui_rules);
        std::string token;
        while (std::getline(ss, token, '\n')) {
            m_codesVector.push_back(token);
        }

        m_codes = MakeCodes();
        m_cylinderHeight *= 1.2f;
        m_cylinderRadius *= 1.3f;
        MakeMatrices();
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
    DrawTree(lightProjection, lightView, m_simpleProgram.get(), m_simpleProgram.get());

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
    
    if(m_scenery) {
        auto skyboxModelTransform =
            glm::translate(glm::mat4(1.0), m_cameraPos) * glm::scale(glm::mat4(1.0), glm::vec3(50.0f));
        m_skyboxProgram->Use();
        m_cubeTexture->Bind();
        m_skyboxProgram->SetUniform("skybox", 0);
        m_skyboxProgram->SetUniform("transform", projection * view * skyboxModelTransform);
        m_box->Draw(m_skyboxProgram.get());
    }

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
    DrawTree(projection, view, m_cylinderProgram.get(), m_leafProgram.get());
    DrawObj(projection, view, m_objProgram.get());
}

// 회전 후 이동 -> 이동행렬 * 회전행렬 (순서)
void Context::DrawTree(const glm::mat4& projection, const glm::mat4& view, const Program* treeProgram, const Program* leafProgram) {
    if(!m_codes.empty()){
        glEnable(GL_BLEND);
        treeProgram->Use();
        treeProgram->SetUniform("tex", 0);
        // m_brownTexture->Bind();
        m_treeTexture->Bind();
        for(int i=0; i<m_modelMatrices.size(); i++){
            auto transform = projection * view * m_modelMatrices[i] * glm::translate(glm::mat4(1.0f),
                glm::vec3(0.0f, -1.0f * m_cylinderHeight, 0.0f));
            treeProgram->SetUniform("transform", transform);
            // treeProgram->SetUniform("color", glm::vec3(0.6f, 0.4f, 0.2f));
            // treeProgram->SetUniform("modelTransform", m_modelMatrices[i]);
            m_cylinder->Draw(treeProgram);
        } 

        leafProgram->Use();
        leafProgram->SetUniform("tex", 0);
        m_treeTexture->Bind();
        for(int i=0; i<m_leafMatrices.size(); i++){
            leafProgram->SetUniform("transform", projection * view * m_leafMatrices[i]);
            // leafProgram->SetUniform("color", glm::vec3(0.2f, 0.6f, 0.2f));
            // leafProgram->SetUniform("modelTransform", m_leafMatrices[i]);
            m_leaf->Draw(leafProgram);
        }

        // m_cylinderInstance = VertexLayout::Create();
        // m_cylinderInstance->Bind();
        // m_cylinder->GetVertexBuffer()->Bind();
        // m_cylinderInstance->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        // m_cylinderPosBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
        //     m_modelMatrices.data(), sizeof(glm::mat4), m_modelMatrices.size());
        // m_cylinderPosBuffer->Bind();
        // m_cylinderInstance->SetAttrib(1, 16, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), 0);
        // glVertexAttribDivisor(1, 1);
        // m_cylinder->GetIndexBuffer()->Bind();

        // glEnable(GL_BLEND);
        // program->Use();
        // m_branchMaterial->SetToProgram(program);
        // m_cylinderInstance->Bind();
        // auto transform = projection * view;
        // program->SetUniform("color", glm::vec3(0.6f, 0.4f, 0.2f));
        // program->SetUniform("transform", transform);
        // glDrawElementsInstanced(GL_TRIANGLES, m_cylinder->GetIndexBuffer()->GetCount(),
        //     GL_UNSIGNED_INT, 0, m_cylinderPosBuffer->GetCount());
    }
}

std::string Context::MakeCodes() {
    std::string result = m_gui_axiom;

    if(m_stochastic) {
        std::string new_str;
        std::default_random_engine rng(std::random_device{}());
        for (int i = 0; i < m_codesVector.size(); i += 2) {
            std::string tmp1 = m_codesVector.at(i);
            std::string tmp2 = m_codesVector.at(i + 1);

            std::size_t pos1 = tmp1.rfind('=');
            std::size_t pos2 = tmp2.rfind('=');

            std::string condition1 = tmp1.substr(0, pos1);
            std::string replace1 = tmp1.substr(pos1 + 1);
            std::string condition2 = tmp2.substr(0, pos2);
            std::string replace2 = tmp2.substr(pos2 + 1);

            std::uniform_real_distribution<double> dist(0.0, 1.0);
            for(int j = 0; j < m_iteration; j++) {
                size_t pos = 0;
                while ((pos = result.find(condition1, pos)) != std::string::npos) {
                    double prob = dist(rng);
                    if (prob < 0.5) {
                        new_str = replace1;
                    } else {
                        new_str = replace2;
                    }
                    result.replace(pos, condition1.length(), new_str);
                    pos += new_str.length();
                }
            }
        }
    }
    else {
        std::string tmp;
        for(int i=0; i<m_codesVector.size(); i++){
            tmp = m_codesVector.at(i);
            std::size_t pos = tmp.rfind('=');

            std::string condition = tmp.substr(0, pos);
            std::string replace = tmp.substr(pos + 1);
            
            // 이 부분에서 한번 iteration이 돌아갈 때마다 무작위로 문자 치환을 하고싶음
            for(int j=0; j<m_iteration; j++)
                result = std::regex_replace(result, std::regex(condition), replace);
        }
    }
    // SPDLOG_INFO("string: {}", result);
    return result;
}

void Context::Clear() {
    // m_stochastic = false;
    m_codes.clear();
    strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "");
    strcpy_s(m_gui_rules, sizeof(m_gui_rules), "");
    m_currentItem = CUSTOM_RULES;
}

void Context::OpenObject(ImGui::FileBrowser file) {
    m_model.reset();
    std::string selected = file.GetSelected().string();
    std::size_t pos = selected.rfind('.');
    std::string tex = selected.substr(0,pos);

    SPDLOG_INFO("File location : {}", selected);
    m_model = Model::Load(selected);
    if(!m_model) {
        // 모델이 생성되지 않았을 때 처리하는 코드
        SPDLOG_ERROR("Failed to open obj : {}", selected);
        m_model.reset();
        return;
    }

    m_floor = false;
    // 동일한 이름의 텍스쳐 파일이 있을 경우 텍스쳐 지정
    if(std::filesystem::exists(tex+".png")) {
        tex+=".png";
        m_modelTexture = Texture::CreateFromImage(Image::Load(tex, false).get());
    }
    else if(std::filesystem::exists(tex+".jpg")) {
        tex+=".jpg";
        m_modelTexture = Texture::CreateFromImage(Image::Load(tex, false).get());
    }
    else {
        m_modelTexture = Texture::CreateFromImage(Image::CreateSingleColorImage(4, 4, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).get());
    }

    Clear();
}

void Context::SaveObject(ImGui::FileBrowser file) {
    if(m_codes.empty()) {
        SPDLOG_ERROR("Create tree object before saving *.obj");
        return;
    }
    std::string selected = file.GetSelected().string();
    std::string filename = "tree";
    std::string num = "1";

    if(std::filesystem::exists(selected + "\\" + filename + ".obj")) {
        int i = 2;
        while(std::filesystem::exists(selected + "\\" + filename + "(" + num + ")" + ".obj")) {
            num = std::to_string(i);
            i++;
        }
        filename += "(" + num + ")" +".obj";
    }
    else{
        filename += ".obj";
    }

    std::ofstream out(selected + "\\" + filename);
    if(WriteToFile(out))
        SPDLOG_INFO("File saved : {}", selected + "\\" + filename);
    out.close();
}

bool Context::WriteToFile(std::ofstream& out) {
    if (!out.is_open()) {
        SPDLOG_ERROR("Failed to open file : {}", std::to_string(out.tellp()));
        return false;
    }
    std::vector<Vertex> modelVertex = m_cylinder->GetVertexVector();
    std::vector<int> modelIndex = m_cylinder->GetIndexVector();
    
    int stride = m_cylinder->GetVertexBuffer()->GetCount();
    std::vector<std::tuple<float, float, float>> v;
    std::vector<std::tuple<float, float>> vt;
    std::vector<std::tuple<float, float, float>> vn;
    std::vector<std::tuple<int, int, int>> f;

    for(int i=0; i<m_modelMatrices.size(); i++) {
        int start = stride * i;
        // vertex positions
        auto matrix = m_modelMatrices[i] * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * m_cylinderHeight, 0.0f));
        for (const auto& vertex : modelVertex) {
            auto vertexAffine = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);
            auto normalAffine = glm::vec4(vertex.normal.x, vertex.normal.y, vertex.normal.z, 0);
            vertexAffine = matrix * vertexAffine;
            normalAffine = matrix * normalAffine;

            v.push_back(std::tuple<float, float, float>(vertexAffine.x, vertexAffine.y, vertexAffine.z));
            vt.push_back(std::tuple<float, float>(vertex.texCoord.x, vertex.texCoord.y));
            vn.push_back(std::tuple<float, float, float>(normalAffine.x, normalAffine.y, normalAffine.z));
        }

        // faces using indices
        size_t indexCount = modelIndex.size();
        for (size_t j = 0; j < indexCount; j += 3) {
            uint32_t index1 = modelIndex[j] + start + 1;
            uint32_t index2 = modelIndex[j+1] + start + 1;
            uint32_t index3 = modelIndex[j+2] + start + 1;

            f.push_back(std::tuple<int, int, int>(index1, index2, index3));
        }
    }

    out << "# tree generator\n";
    out << "o Cylinder\n";
    out << "# vertex coordinates\n";
    for(int i=0; i<v.size(); i++) {
        out << "v " << std::get<0>(v[i]) << " " << std::get<1>(v[i]) << " " << std::get<2>(v[i]) << "\n";
    }

    out << "\n# texture coordinates\n";
    for(int i=0; i<vt.size(); i++) {
        out << "vt " << std::get<0>(vt[i]) << " " << std::get<1>(vt[i]) << "\n";
    }

    out << "\n# normal coordinates\n";
    for(int i=0; i<vn.size(); i++) {
        out << "vn " << std::get<0>(vn[i]) << " " << std::get<1>(vn[i]) << " " << std::get<2>(vn[i]) << "\n";
    }

    out << "\n# face\n";
    for(int i=0; i<f.size(); i++) {
        out << "f "
                << std::get<0>(f[i]) << "/" << std::get<0>(f[i]) << "/" << std::get<0>(f[i]) << " "
                << std::get<1>(f[i]) << "/" << std::get<1>(f[i]) << "/" << std::get<1>(f[i]) << " "
                << std::get<2>(f[i]) << "/" << std::get<2>(f[i]) << "/" << std::get<2>(f[i])
                << "\n";
    }

    v.clear();
    vt.clear();
    vn.clear();
    f.clear();

    std::vector<Vertex> leafVertex = m_leaf->GetVertexVector();
    std::vector<int> leafIndex = m_leaf->GetIndexVector();
    int leafStart = stride * m_modelMatrices.size();
    
    int leafStride = m_leaf->GetVertexBuffer()->GetCount();
    for(int i = 0; i<m_leafMatrices.size(); i++) {
        int start = leafStride * i;
        // vertex positions
        auto matrix = m_leafMatrices[i];
        for (const auto& vertex : leafVertex) {
            auto vertexAffine = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);
            auto normalAffine = glm::vec4(vertex.normal.x, vertex.normal.y, vertex.normal.z, 0);
            vertexAffine = matrix * vertexAffine;
            normalAffine = matrix * normalAffine;

            v.push_back(std::tuple<float, float, float>(vertexAffine.x, vertexAffine.y, vertexAffine.z));
            vt.push_back(std::tuple<float, float>(vertex.texCoord.x, vertex.texCoord.y));
            vn.push_back(std::tuple<float, float, float>(normalAffine.x, normalAffine.y, normalAffine.z));
        }

        // faces using indices
        size_t indexCount = leafIndex.size();
        for (size_t j = 0; j < indexCount; j += 3) {
            uint32_t index1 = leafIndex[j] + leafStart + start + 1;
            uint32_t index2 = leafIndex[j+1] + leafStart +start + 1;
            uint32_t index3 = leafIndex[j+2] + leafStart + start + 1;

            f.push_back(std::tuple<int, int, int>(index1, index2, index3));
        }
    }

    out << "\no Leaf\n";
    out << "# vertex coordinates\n";
    for(int i=0; i<v.size(); i++) {
        out << "v " << std::get<0>(v[i]) << " " << std::get<1>(v[i]) << " " << std::get<2>(v[i]) << "\n";
    }

    out << "\n# texture coordinates\n";
    for(int i=0; i<vt.size(); i++) {
        out << "vt " << std::get<0>(vt[i]) << " " << std::get<1>(vt[i]) << "\n";
    }

    out << "\n# normal coordinates\n";
    for(int i=0; i<vn.size(); i++) {
        out << "vn " << std::get<0>(vn[i]) << " " << std::get<1>(vn[i]) << " " << std::get<2>(vn[i]) << "\n";
    }

    out << "\n# face\n";
    for(int i=0; i<f.size(); i++) {
        out << "f "
                << std::get<0>(f[i]) << "/" << std::get<0>(f[i]) << "/" << std::get<0>(f[i]) << " "
                << std::get<1>(f[i]) << "/" << std::get<1>(f[i]) << "/" << std::get<1>(f[i]) << " "
                << std::get<2>(f[i]) << "/" << std::get<2>(f[i]) << "/" << std::get<2>(f[i])
                << "\n";
    }

    return true;
}

void Context::DrawObj(const glm::mat4& projection, const glm::mat4& view, const Program* program) {
    if(m_model) {
        program->Use();
        program->SetUniform("tex", 0);
        m_modelTexture->Bind();
        auto transform = projection * view;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", glm::mat4(1.0f));
        m_model->Draw(program);
    }
}

// context.cpp
void Context::DrawScene(const glm::mat4& projection, const glm::mat4& view, const Program* program) {
    // 바닥
    if(m_floor){
        program->Use();
        auto modelTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
        auto transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_planeMaterial->SetToProgram(program);
        m_box->Draw(program);
    }
}

// 회전 후 이동 -> 이동행렬 * 회전행렬 (순서)
void Context::MakeMatrices() {
    // 나뭇잎을 생성하는 위치를 결정하는 벡터
    std::vector<glm::mat4> leafMatrices;
    float randomAngle = 0.0f;
    glm::mat4 scalingInverse;
    auto drawLeaves = [&](glm::mat4 matrices, glm::mat4 scaling, char direction)-> void {
        // double trigonTranslate = m_heightScaling * sin(randomAngle * M_PI / 180.0f) * (m_leafHeight / 2.0f);
        // double trigonDown = (-1.0f) * (cos(randomAngle * M_PI / 180.0f) * (2.0f * m_leafRadius)
        //     + (1.0f - m_heightScaling) / 2.0f * m_leafHeight);

        // glm::mat4 rotateYMinus = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        // glm::mat4 rotateYPlus = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        // glm::mat4 rotateY180 = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // glm::mat4 rotateXMinus = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        // glm::mat4 rotateXPlus = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(1.0f, 0.0f, 0.0f));

        // glm::mat4 rotateZMinus = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        // glm::mat4 rotateZPlus = 
        //     glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(0.0f, 0.0f, 1.0f));

        // glm::mat4 rotate;
        // switch(direction){
        // case 'F': case 'X': case 'A': case 'C':
        //     break;

        // case '+':
        //     rotate = rotateYPlus;
        //     break;
        
        // case '-':
        //     rotate = rotateYMinus;
        //     break;
        
        // case '^':
        //     rotate = rotateXPlus;
        //     break;

        // case '&':
        //     rotate = rotateXMinus;
        //     break;

        // case '<':
        //     rotate = rotateZPlus;
        //     break;

        // case '>':
        //     rotate = rotateZMinus;
        //     break;

        // case '|':
        //     rotate = rotateY180;
        //     break;
        // }
        auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight / -2.0f, 0.0f));
        
        leafMatrices.push_back(matrices * translate * scaling);
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> normalDistFrontGen(0.0f, 2.0f);
    std::normal_distribution<float> normalDistEndGen(0.0f, 0.5f);
    std::normal_distribution<float> normalDistAngle(m_angle, 2.0f);
    std::uniform_real_distribution<float> uniformDist(m_heightScaling - 0.05f, m_heightScaling + 0.05f);

    // 나뭇가지를 생성하는 위치를 결정하는 코드
    MatrixStack stack; // 행렬 연산을 위한 스택
    std::stack<int> stackCount; // pop 하는 수를 정하기 위한 스택

    std::stack<char> direction; // 나뭇잎의 방향을 정하기 위한 스택
    std::stack<int> directionCount; // 나뭇잎 방향 pop 하는 수 정하기 위한 스택

    MatrixStack scalingStack; // 나뭇잎 크기 계산을 위함
    std::stack<int> scalingCount;

    stack.pushMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight/1.7f, 0.0f)));
    stackCount.push(0);
    directionCount.push(0);
    scalingCount.push(0);

    int randomNum;
    float weight = 1.5f;
    std::vector<glm::mat4> modelMatrices;

    int matrixTop = 0;
    int directionTop = 0;
    int scalingTop = 0;
    auto matrixFunction = [&]()-> void {
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
        randomAngle = normalDistAngle(gen);
        switch(m_codes.at(i)){
        case 'F': case 'X': case 'A': case 'C':
            matrixTop = stackCount.top();
            matrixTop+=1;
            stackCount.pop();
            stackCount.push(matrixTop);
            // glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * m_cylinderHeight, 0.0f));
            stack.pushMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(m_radiusScaling, m_heightScaling, m_radiusScaling)) *
                glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight * (m_heightScaling + 1.0f) / 2.2f, 0.0f))); // 방향
            modelMatrices.push_back(stack.getCurrentMatrix());

            scalingTop = scalingCount.top();
            scalingTop+=1;
            scalingCount.pop();
            scalingCount.push(scalingTop);
            // 역행렬이 무조건 존재한다고 가정
            scalingInverse = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / m_radiusScaling,
                1.0f / m_heightScaling, 1.0f / m_radiusScaling));
            scalingStack.pushMatrix(scalingInverse);

            randomNum = static_cast<int>(floor((normalDistFrontGen(gen))));
            if(randomNum == 0 && !direction.empty() && !stack.isEmpty() && !scalingStack.isEmpty())
                drawLeaves(stack.getCurrentMatrix(), scalingStack.getCurrentMatrix(), direction.top());
            break;

        case '+':
            matrixFunction();
            direction.push('+');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(0.0f, 1.0f, 0.0f))); // 방향
            break;

        case '-':
            matrixFunction();
            direction.push('-');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(0.0f, 1.0f, 0.0f))); // 방향
            break;

        case '^':
            matrixFunction();
            direction.push('^');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(1.0f, 0.0f, 0.0f)) *
                glm::translate(glm::mat4(1.0f), glm::vec3(
                    0.0f, 0.0f, weight * sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f)))); // 방향
            break;

        case '&':
            matrixFunction();
            direction.push('&');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * 
                glm::translate(glm::mat4(1.0f),glm::vec3(
                0.0f, 0.0f,-1.0 * weight *sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f)))); // 방향
            break;

        case '<':
            matrixFunction();
            direction.push('<');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::translate(glm::mat4(1.0f), glm::vec3(
                -1.0 * weight * sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f), 0.0f, 0.0f))); // 방향
            break;

        case '>':
            matrixFunction();
            direction.push('>');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * 
                glm::translate(glm::mat4(1.0f), glm::vec3(
                weight * sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f), 0.0f, 0.0f))); // 방향
            break;

        case '|':
            matrixFunction();
            direction.push('|');
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))); // 방향
            break;

        case '[':
            stackCount.push(0);
            directionCount.push(0);
            scalingCount.push(0);
            break;

        case ']':
            if(m_codes.at(i-1) == 'X' || m_codes.at(i-1) == 'F' || m_codes.at(i-1) == 'A' || m_codes.at(i-1) == 'C') {
                drawLeaves(stack.getCurrentMatrix(), scalingStack.getCurrentMatrix(), direction.top());
            }
                
            for(int i=0; i<stackCount.top(); i++)
                stack.popMatrix();
            for(int i=0; i<directionCount.top(); i++)
                direction.pop();
            for(int i=0; i<scalingCount.top(); i++)
                scalingStack.popMatrix();

            stackCount.pop();
            directionCount.pop();
            scalingCount.pop();
            break;
        }
    }
    m_modelMatrices.clear();
    m_leafMatrices.clear();
    m_modelMatrices = modelMatrices;
    m_leafMatrices = leafMatrices;
}

void Context::SetRules() {
    m_stochastic = false;

    switch(m_currentItem) {
    case CUSTOM_RULES:

        break;

    case ARROW_TREE:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "FFA");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules),
            "A=F[--&&&FC][++&&&FC][--^FC][++^FC]\n"
            "C=F[--<&&FC]||[++>&&FC]||[+<^^FC]||[->^^FC]");
        break;

    case STOCHASTIC:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "FFA");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules),
            "A=F[--&&&FFC][++&&&FC][--^FC][++^FFC]\n"\
            "A=F[--&&&FC][++&&&FFC][--^FFC][++^FC]\n"\
            "C=FF[--<&&FC]||[++>&&FFC]||[+<^^FC]||[->^^FFC]\n"\
            "C=F[--<&&FFC]||[++>&&FC]||[+<^^FFC]||[->^^FC]");
        m_stochastic = true;
        break;

    case SKEWED_TREE:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "FFA");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules),
            "A=F[<F[&C][^C]][<F[&&C][&C]][>F[&C][^C]]\n"
            "C=F[--<&&FC]||[++>&&FC]||[+<^^FC]||[->^^FC]");
        break;

    case BINARYTREE:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "X");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules), "X=F[<X][>X]");
        break;

    default:
        break;
    }
}