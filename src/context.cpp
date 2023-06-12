#include "context.h"
#include "image.h"
#include "glm/gtx/string_cast.hpp"
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

    m_objProgram = Program::Create("./shader/obj.vs", "./shader/obj.fs");
    if(!m_objProgram) return false;

    m_normalProgram = Program::Create("./shader/normal.vs", "./shader/normal.fs");
    if(!m_normalProgram) return false;

    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    TexturePtr grayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

    m_planeMaterial = Material::Create();
    m_planeMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/marble.jpg").get());
    m_planeMaterial->specular = grayTexture;
    m_planeMaterial->shininess = 4.0f;

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

    m_lsystem = LSystem::Create("","", m_treeParam, m_angle, 0);
    if(!m_lsystem) return false;

    m_lsystem2 = LSystem::Create("X", "X=F[<X][>X]", m_treeParam, m_angle, 3, 2.0f, true);
    m_lsystem2->Move(-2.0f, -2.0f);

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
        SaveObject(m_fileDialogSave, m_lsystem);
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
            ImGui::SameLine();
            ImGui::Checkbox("floor", &m_floor);
            ImGui::SameLine();
            ImGui::Checkbox("scenery", &m_scenery);
        }
        ImGui::EndChild();
        ImGui::SetWindowPos(m_UIPos);
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
        ImGui::DragFloat("leaf radius", &m_gui_leaf_radius, 0.005f, 0.05f, 0.3f);
        ImGui::DragFloat("leaf length", &m_gui_leaf_height, 0.005f, 0.05f, 0.3f);
        ImGui::Separator();
        ImGui::InputText("axiom", m_gui_axiom, sizeof(m_gui_axiom));
        ImGui::Combo("rules", &m_currentItem, m_comboItems, NUM_RULES);
        ImGui::InputTextMultiline("##rules", m_gui_rules, IM_ARRAYSIZE(m_gui_rules),
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5), ImGuiInputTextFlags_AllowTabInput);
        ImGui::Separator();
        ImGui::DragInt("iteration", &m_iteration, 0.05f, 0, 5);
        ImGui::Checkbox("sphere leaves", &m_sphereLeaves);
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
            ImGui::TextWrapped("%s",m_lsystem->GetCodes().c_str());
        }
        ImGui::EndChild();
        ImGui::EndChild();
        ImGui::SetWindowPos(m_treePos);
        ImGui::End();
    }

    if(m_currentItem != CUSTOM_RULES)
        SetRules();

    auto lightView = glm::lookAt(m_light.position,
        m_light.position + m_light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto lightProjection = m_light.directional ?
        glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f) :
        glm::perspective(glm::radians((m_light.cutoff[0] + m_light.cutoff[1]) * 2.0f), 1.0f, 1.0f, 20.0f);

    if(m_newCodes){
        m_treeParam = { m_cylinderRadius, m_cylinderHeight, m_leafRadius, m_leafHeight, m_radiusScaling, m_heightScaling };
        m_lsystem = LSystem::Create(m_gui_axiom, m_gui_rules, m_treeParam, m_angle, m_iteration, m_sphereLeaves);
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
    DrawTree(projection, view, m_logProgram.get(), m_leafProgram.get());
    DrawObj(projection, view, m_objProgram.get());
}

void Context::SetRules() {
    switch(m_currentItem) {
    case CUSTOM_RULES:

        break;

    case ARROW_TREE:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "FFA");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules),
            "A=F[--&&FC][++&&FC][--^FC][++^FC]\n"
            "C=F[--<&&FC]||[++>&&FC]||[+<^^FC]||[->^^FC]");
        break;

    case STOCHASTIC:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "FFA");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules),
            "A=F++++[&&FC]++++[&&FC]++++[^FC]++++[^FC]\n"\
            "A=F----[&&FC]----[&&FC]----[^FC]----[^FC]\n"\
            "C=|F[--<&&FC]||[++>&&FFC]||[+<^^FC]||[->^^FFC]\n"\
            "C=F[--<&&FFC]||[++>&&FC]||[+<^^FFC]||[->^^FC]");
        break;

    case BUSH_LIKE:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "FFA");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules),
            "A=[C]++++[C]++++[C]\n"\
            "C=&FFA");
        break;

    case BINARYTREE:
        strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "X");
        strcpy_s(m_gui_rules, sizeof(m_gui_rules), "X=F[<X][>X]");
        break;

    default:
        break;
    }
}

// 회전 후 이동 -> 이동행렬 * 회전행렬 (순서)
void Context::DrawTree(const glm::mat4& projection, const glm::mat4& view, const Program* treeProgram, const Program* leafProgram) {
    glEnable(GL_BLEND);
    m_lsystem->Draw(projection, view);
    // m_lsystem2->Draw(projection, view);
}

void Context::Clear() {
    // m_stochastic = false;
    strcpy_s(m_gui_axiom, sizeof(m_gui_axiom), "");
    strcpy_s(m_gui_rules, sizeof(m_gui_rules), "");
    LSystem::Create(m_gui_axiom, m_gui_rules, m_treeParam, m_angle, 0);
    m_currentItem = CUSTOM_RULES;
    m_newCodes = true;
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

void Context::SaveObject(ImGui::FileBrowser file, const LSystemUPtr& tree) {
    if(tree->isEmpty()) {
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
        filename += "(" + num + ")";
    }

    if(WriteToFile(selected, filename, tree))
        SPDLOG_INFO("File saved : {}", selected + "\\" + filename);
}

bool Context::WriteToFile(std::string selected, std::string filename, const LSystemUPtr& tree) {
    std::ofstream outObj(selected + "\\" + filename + ".obj");
    std::ofstream outMtl(selected + "\\" + filename + ".mtl");
    std::string texturePath = selected + "/" + filename + ".png";

    if(!tree->ExportObj(outObj, filename))
        SPDLOG_ERROR("Faile to export file : {}", selected + "\\" + filename + ".obj");
    if(!tree->ExportMtl(outMtl, filename))
        SPDLOG_ERROR("Faile to export file : {}", selected + "\\" + filename + ".mtl");
    // if(!tree->ExportTexture(texturePath.c_str()))
    //     SPDLOG_ERROR("Faile to export file : {}", selected + "\\" + filename + ".png");

    outObj.close();
    outMtl.close();
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

void Context::DrawScene(const glm::mat4& projection, const glm::mat4& view, const Program* program) {
    // 바닥
    if(m_floor){
        program->Use();
        auto modelTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
        auto transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_planeMaterial->SetToProgram(program);
        m_box->Draw(program);
    }
}
