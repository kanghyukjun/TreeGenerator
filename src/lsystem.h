#ifndef __LSYSTEM_H__
#define __LSYSTEM_H__

#include "common.h"
#include "matrix_stack.h"
#include "program.h"
#include "mesh.h"
#include "texture.h"
#include <regex>
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <random>
#include <string>
#include <vector>
#include <fstream>


// "이동"에 사용되는 문자 : F, X, A, C
CLASS_PTR(LSystem);
class LSystem {
public:
    // std::vector {cylinderRadius, cylinderHeight, leafRadius, leafHeight, radiusScaling, heightScaling}
    static LSystemUPtr Create(std::string axiom, std::string rules, std::vector<float> treeParam, float angle, int iteration,
        float xCoord = 0.0f, float zCoord = 0.0f);
    std::string GetAxiom() { return m_axiom; }
    std::string GetRules() { return m_rules; }
    std::string GetCodes() { return m_codes; }
    bool isEmpty() { return m_codes.empty(); }
    void Draw(const glm::mat4& projection, const glm::mat4& view, const Program* treeProgram, const Program* leafProgram) const;
    void Move(float xCoord, float zCoord);
    bool ExportObj(std::ofstream& out);

private:
    LSystem() {};
    bool Init(std::string axiom, std::string rules, std::vector<float> treeParam, float angle, int iteration,
        float zCoord, float xCoord);
    std::string MakeCodes();
    void MakeCylinderMatrices(float xCoord = 0.0f, float zCoord = 0.0f);
    void MakeLeafMatrices(glm::mat4 matrices, glm::mat4 scaling, std::vector<glm::mat4>& vector);

    MeshUPtr m_log;
    MeshUPtr m_leaf;

    TexturePtr m_leafTexture;
    TexturePtr m_treeTexture;

    std::vector<glm::mat4> m_cylinderVector;
    std::vector<glm::mat4> m_leafVector;
    std::string m_axiom;
    std::string m_rules;

    float m_cylinderRadius;
    float m_cylinderHeight;
    float m_leafRadius;
    float m_leafHeight;
    float m_radiusScaling;
    float m_heightScaling;

    float m_angle;
    int m_iteration;

    float m_xCoord;
    float m_zCoord;

    std::vector<std::string> m_codesVector;
    std::string m_codes;
};

#endif //__LSYSTEM_H__