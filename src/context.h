#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"
#include "mesh.h"
#include "model.h"
#include "framebuffer.h"
#include "shadow_map.h"
#include "matrix_stack.h"
#include "lsystem.h"
#include <imgui.h>
#include "imfilebrowser.h"

CLASS_PTR(Context)
class Context{
public:
    static ContextUPtr Create();
    void Render();
    void ProcessInput(GLFWwindow* window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);

    void DrawScene(const glm::mat4& projection, const glm::mat4& view, const Program* program);
    void DrawObj(const glm::mat4& projection, const glm::mat4& view, const Program* program);
    void DrawTree(const glm::mat4& projection, const glm::mat4& view, const Program* treeProgram, const Program* leafProgram);

private:
    Context(){}
    bool Init();
    void Clear();
    void OpenObject(ImGui::FileBrowser file);
    void SaveObject(ImGui::FileBrowser file);
    bool WriteToFile(std::ofstream& out);
    void SetRules();

    ProgramUPtr m_program;
    ProgramUPtr m_simpleProgram;
    ProgramUPtr m_textureProgram;
    ProgramUPtr m_postProgram;
    ProgramUPtr m_objProgram;
    MeshUPtr m_box;

    // tree program
    ProgramUPtr m_leafProgram;
    ProgramUPtr m_logProgram;


    // material parameter
    MaterialPtr m_planeMaterial;
    MaterialPtr m_box1Material;
    MaterialPtr m_branchMaterial;
    MaterialPtr m_leafMaterial;
    MaterialPtr m_objMaterial;

    // framebuffer
    FramebufferUPtr m_framebuffer;

    ModelUPtr m_model;
    TexturePtr m_modelTexture;

    // cubemap
    CubeTextureUPtr m_cubeTexture;
    ProgramUPtr m_skyboxProgram;
    ProgramUPtr m_envMapProgram;

    // shadow map
    ShadowMapUPtr m_shadowMap;
    ProgramUPtr m_lightingShadowProgram;

    // tree
    bool m_newCodes { false };
    int m_iteration { 3 };

    float m_cylinderRadius { 0.1f };
    float m_cylinderHeight { 1.0f };
    float m_leafRadius { 0.2f };
    float m_leafHeight { 0.2f };
    float m_radiusScaling { 0.75f };
    float m_heightScaling { 0.75f };
    float m_angle { 30.0f };
    std::vector<float> m_treeParam { m_cylinderRadius, m_cylinderHeight, m_leafRadius, m_leafHeight, m_radiusScaling, m_heightScaling };
    LSystemUPtr m_lsystem;
    LSystemUPtr m_lsystem2;

    // light parameter
    struct Light {
        bool directional { true };
        glm::vec3 position { glm::vec3(0.0f, 15.0f, 1.0f) };
        glm::vec3 direction { glm::vec3(2.0f, -7.0f, -1.0f) };
        glm::vec2 cutoff { glm::vec2(50.0f, 5.0f) };
        float distance { 150.0f };
        glm::vec3 ambient { glm::vec3(0.3f, 0.3f, 0.3f) };
        glm::vec3 diffuse { glm::vec3(0.5f, 0.5f, 0.5f) };
        glm::vec3 specular { glm::vec3(1.0f, 1.0f, 1.0f) };
    };
    Light m_light;
    bool m_blinn { false };

    // clear color
    glm::vec4 m_clearColor { glm::vec4(0.1f, 0.2f, 0.3f, 0.0f) };

    // camera parameter
    bool m_cameraControl { false };
    glm::vec2 m_prevMousePos { glm::vec2(0.0f) };
    float m_cameraPitch { -14.0f };
    float m_cameraYaw { 0.0f };
    glm::vec3 m_cameraPos { glm::vec3(0.0f, 4.0f, 12.0f) };
    glm::vec3 m_cameraFront { glm::vec3(0.0f, 0.0f, -1.0f) };
    glm::vec3 m_cameraUp { glm::vec3(0.0f, 1.0f, 0.0f) };
    bool m_floor { true };
    bool m_scenery { true };

    // tree gui
    float m_gui_angle { m_angle };
    float m_gui_radius { m_cylinderRadius };
    float m_gui_length { m_cylinderHeight };
    float m_gui_leaf_radius { m_leafRadius };
    float m_gui_leaf_height { m_leafHeight };
    char m_gui_axiom[1024] = { "FFA" }; 
    char m_gui_rules[1024 * 4] = {
        "A=F[--&&&FC][++&&&FC][--^FC][++^FC]\n"
        "C=F[--<&&FC]||[++>&&FC]||[+<^^FC]||[->^^FC]" };
    std::string m_axiom { m_gui_axiom };
    std::string m_rules { m_gui_rules };

    enum Rule {
        CUSTOM_RULES,
        ARROW_TREE,
        STOCHASTIC,
        BUSH_LIKE,
        BINARYTREE,
        NUM_RULES
    };
    char* m_comboItems[NUM_RULES] { "custom rules", "arrow tree", "stochastic", "bush-like", "binary tree" };
    int m_currentItem = ARROW_TREE;

    ImGui::FileBrowser m_fileDialogOpen;
    ImGui::FileBrowser m_fileDialogSave {ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_EnterNewFilename};

    ImVec2 m_UIPos { 3.0f, 25.0f };
    ImVec2 m_treePos { 1241.0f, 25.0f };

    int m_width { WINDOW_WIDTH };
    int m_height { WINDOW_HEIGHT };
};

#endif // __CONTEXT_H__