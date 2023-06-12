#include "lsystem.h"

LSystemUPtr LSystem::Create(std::string axiom, std::string rules, std::vector<float> treeParam, float angle, int iteration,
        bool sphere, float xCoord, float zCoord) {
    auto lsystem = LSystemUPtr(new LSystem());
    if(!lsystem->Init(axiom, rules, treeParam, angle, iteration, sphere, xCoord, zCoord))
        return nullptr;
    
    return std::move(lsystem);
}

bool LSystem::Init(std::string axiom, std::string rules, std::vector<float> treeParam, float angle, int iteration,
    bool sphere, float xCoord, float zCoord) {
    if(treeParam.size() < 6) return false;
    else if(treeParam[4] <= 0.0f && treeParam[5] <= 0.0f) return false;

    m_codesVector.clear();
    m_axiom = axiom;
    m_rules = rules;
    m_cylinderRadius = treeParam[0];
    m_cylinderHeight = treeParam[1];
    m_leafRadius = treeParam[2];
    m_leafHeight = treeParam[3];
    m_radiusScaling = treeParam[4];
    m_heightScaling = treeParam[5];

    m_angle = angle;
    m_iteration = iteration;
    m_isSphere = sphere;

    m_xCoord = xCoord;
    m_zCoord = zCoord;

    std::istringstream ss(rules);
    std::string token;
    while (std::getline(ss, token, '\n')) {
        m_codesVector.push_back(token);
    }

    m_codes = MakeCodes();
    // m_cylinderHeight *= 1.2f;
    // m_cylinderRadius *= 1.3f;
    MakeCylinderMatrices(m_xCoord, m_zCoord);

    m_log = Mesh::CreateCylinder(m_cylinderRadius, m_cylinderHeight, m_radiusScaling);
    m_leaf = Mesh::CreateLeaf(m_leafRadius, m_leafHeight);
    m_sphere = Mesh::CreateSphere(m_leafRadius);

    m_leafTexture = Texture::CreateFromImage(Image::Load("./image/leaf2.png").get());
    m_greenTexture = Texture::CreateFromImage(Image::CreateSingleColorImage(4, 4, glm::vec4(0.27f, 0.334f, 0.118f, 1.0f)).get());
    m_treeImage = Image::Load("./image/tree.png");
    m_treeTexture = Texture::CreateFromImage(m_treeImage.get());

    m_logProgram = Program::Create("./shader/cylinder.vs", "./shader/cylinder.fs");
    if(!m_logProgram) return false;

    m_leafProgram = Program::Create("./shader/leaf.vs", "./shader/leaf.fs");
    if(!m_leafProgram) return false;

    return true;
}

std::string LSystem::MakeCodes() {
    std::random_device rd; // 시드로 사용할 장치
    std::mt19937 gen(rd()); // 난수 엔진

    std::string tmp;

    std::vector<std::string> F;
    std::vector<std::string> X;
    std::vector<std::string> A;
    std::vector<std::string> C;

    bool FCheck = false;
    bool XCheck = false;
    bool ACheck = false;
    bool CCheck = false;
    
    // 각 벡터에 변환 규칙을 삽입
    for(int i=0; i<m_codesVector.size(); i++) {
        tmp = m_codesVector[i];
        std::size_t pos = tmp.rfind('=');
        if(pos == std::string::npos) continue;

        std::string condition = tmp.substr(0, pos);
        std::string replace = tmp.substr(pos + 1);

        if(condition.compare("F") == 0)
            F.push_back(replace);
        else if(condition.compare("X") == 0)
            X.push_back(replace);
        else if(condition.compare("A") == 0)
            A.push_back(replace);
        else if(condition.compare("C") == 0)
            C.push_back(replace);
    }

    std::string result = m_axiom; // 치환될 문자열
    std::string new_str; // 규칙 문자열

    auto StringCheck = [] (char replace, std::string str) -> bool {
        if(str.find(replace) == std::string::npos) return false;
        return true;
    };

    auto ReplaceString = [&result, &new_str, &gen] (char replace, std::vector<std::string> vector, bool check) {
        size_t pos = 0;
        while ((pos = result.find(replace, pos)) != std::string::npos) {
            if(vector.empty() || !check) break;
            else if(vector.size() == 1) {
                new_str = vector[0];
            }
            else {
                std::uniform_int_distribution<> dis(0, vector.size() - 1); // 범위 설정
                int randomIndex = dis(gen);
                new_str = vector[randomIndex];
            }
            result.replace(pos, 1, new_str);
            pos += new_str.length();
        }
    };

    for(int i = 0; i < m_iteration; i++) {
        FCheck = StringCheck('F', result);
        XCheck = StringCheck('X', result);
        ACheck = StringCheck('A', result);
        CCheck = StringCheck('C', result);

        ReplaceString('F', F, FCheck);
        ReplaceString('X', X, XCheck);
        ReplaceString('A', A, ACheck);
        ReplaceString('C', C, CCheck);
    }

    return result;
}

void LSystem::MakeLeafMatrices(glm::mat4 matrices, glm::mat4 scaling, std::vector<glm::mat4>& vector) {
    auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight / -2.0f, 0.0f));
    vector.push_back(matrices * translate * scaling);
}

// 회전 후 이동 -> 이동행렬 * 회전행렬 (순서)
void LSystem::MakeCylinderMatrices(float xCoord, float zCoord) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> normalDistFrontGen(0.0f, 2.0f);
    std::normal_distribution<float> normalDistEndGen(0.0f, 0.5f);
    std::normal_distribution<float> normalDistAngle(m_angle, 4.0f);
    std::uniform_real_distribution<float> uniformDist(m_heightScaling - 0.05f, m_heightScaling + 0.05f);

    // 나뭇가지를 생성하는 위치를 결정하는 코드
    MatrixStack stack(xCoord, zCoord); // 행렬 연산을 위한 스택
    std::stack<int> stackCount; // pop 하는 수를 정하기 위한 스택

    MatrixStack scalingStack; // 나뭇잎 크기 계산을 위함
    std::stack<int> scalingCount;

    // stack.pushMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_cylinderHeight/1.7f, 0.0f)));
    stackCount.push(0);
    scalingCount.push(0);

    int randomNum;
    float weight = 1.5f;
    std::vector<glm::mat4> modelMatrices;
    std::vector<glm::mat4> leafMatrices;

    int matrixTop = 0;
    int directionTop = 0;
    int scalingTop = 0;
    auto matrixFunction = [&]()-> void {
        matrixTop = stackCount.top();
        matrixTop+=1;
        stackCount.pop();
        stackCount.push(matrixTop);
    };

    float randomAngle = 0.0f;
    glm::mat4 scalingInverse;

    auto coord = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 3.0f));
    for(int i=0; i<m_codes.length(); i++){
        randomAngle = normalDistAngle(gen);
        switch(m_codes.at(i)){
        case 'F': case 'X': case 'A': case 'C':
            matrixFunction();
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

            // randomNum = static_cast<int>(floor((normalDistFrontGen(gen))));
            // if(randomNum == 0 && !stack.isEmpty() && !scalingStack.isEmpty())
            //     MakeLeafMatrices(stack.getCurrentMatrix(), scalingStack.getCurrentMatrix(), leafMatrices);
            break;

        case '+':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(0.0f, 1.0f, 0.0f))); // 방향
            break;

        case '-':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(0.0f, 1.0f, 0.0f))); // 방향
            break;

        case '^':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(1.0f, 0.0f, 0.0f)) *
                glm::translate(glm::mat4(1.0f), glm::vec3(
                    0.0f, 0.0f, weight * sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f)))); // 방향
            break;

        case '&':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * 
                glm::translate(glm::mat4(1.0f),glm::vec3(
                0.0f, 0.0f,-1.0 * weight *sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f)))); // 방향
            break;

        case '<':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(randomAngle), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::translate(glm::mat4(1.0f), glm::vec3(
                -1.0 * weight * sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f), 0.0f, 0.0f))); // 방향
            break;

        case '>':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * randomAngle), glm::vec3(0.0f, 0.0f, 1.0f)) * 
                glm::translate(glm::mat4(1.0f), glm::vec3(
                weight * sin(randomAngle * M_PI / 180.0f) * (m_cylinderHeight/2.0f), 0.0f, 0.0f))); // 방향
            break;

        case '|':
            matrixFunction();
            stack.pushMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))); // 방향
            break;

        case '[':
            stackCount.push(0);
            scalingCount.push(0);
            break;

        case ']':
            randomNum = static_cast<int>(floor((normalDistEndGen(gen))));
            if((m_codes.at(i-1) == 'X' || m_codes.at(i-1) == 'F' || m_codes.at(i-1) == 'A' || m_codes.at(i-1) == 'C')
                && randomNum == 0 || randomNum == -1) {
                MakeLeafMatrices(stack.getCurrentMatrix(), scalingStack.getCurrentMatrix(), leafMatrices);
            }
                
            for(int i=0; i<stackCount.top(); i++)
                stack.popMatrix();
            for(int i=0; i<scalingCount.top(); i++)
                scalingStack.popMatrix();

            stackCount.pop();
            scalingCount.pop();
            break;
        }
    }
    m_cylinderVector.clear();
    m_leafVector.clear();
    m_cylinderVector = modelMatrices;
    m_leafVector = leafMatrices;
}

void LSystem::Draw(const glm::mat4& projection, const glm::mat4& view) const {
    if(!m_codes.empty()) {
        m_logProgram->Use();
        m_logProgram->SetUniform("tex", 0);
        // m_brownTexture->Bind();
        m_treeTexture->Bind();

        for(int i=0; i<m_cylinderVector.size(); i++) {
            auto transform = projection * view * m_cylinderVector[i] * glm::translate(glm::mat4(1.0f),
                glm::vec3(0.0f, -1.0f * m_cylinderHeight, 0.0f));
            m_logProgram->SetUniform("transform", transform);
            // m_logProgram->SetUniform("color", glm::vec3(0.6f, 0.4f, 0.2f));
            // treeProgram->SetUniform("modelTransform", m_modelMatrices[i]);
            m_log->Draw(m_logProgram.get());
        }

        m_leafProgram->Use();
        m_leafProgram->SetUniform("tex", 0);
        if(m_isSphere) {
            m_greenTexture->Bind();
            for(int i=0; i<m_leafVector.size(); i++){
                m_leafProgram->SetUniform("transform", projection * view * m_leafVector[i]);
                m_sphere->Draw(m_leafProgram.get());
            }
        }
        else {
            m_treeTexture->Bind();
            for(int i=0; i<m_leafVector.size(); i++){
                m_leafProgram->SetUniform("transform", projection * view * m_leafVector[i]);
                m_leaf->Draw(m_leafProgram.get());
            }
        }
    }
}

void LSystem::Move(float xCoord, float zCoord) {
    if(xCoord == m_xCoord && zCoord == m_zCoord) return;

    m_xCoord = xCoord;
    m_zCoord = zCoord;
    MakeCylinderMatrices(xCoord, zCoord);
}

bool LSystem::ExportObj(std::ofstream& out, std::string material) {
    if (!out.is_open()) {
        SPDLOG_ERROR("Failed to open file : {}", std::to_string(out.tellp()));
        return false;
    }

    int stride = m_log->GetVertexBuffer()->GetCount();
    std::vector<Vertex> modelVertex = m_log->GetVertexVector();
    std::vector<int> modelIndex = m_log->GetIndexVector();

    std::vector<std::tuple<float, float, float>> v;
    std::vector<std::tuple<float, float>> vt;
    std::vector<std::tuple<float, float, float>> vn;
    std::vector<std::tuple<int, int, int>> f;

    for(int i=0; i<m_cylinderVector.size(); i++) {
        int start = stride * i;
        // vertex positions
        auto matrix = m_cylinderVector[i] * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * m_cylinderHeight, 0.0f));
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

    out << "# tree generator\n\n";
    out << "# material\n";
    out << "mtllib ./" + material + ".mtl\n";

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
    out << "usemtl Tree\n";
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

    if(m_isSphere) {
        std::vector<Vertex> sphereVertex = m_sphere->GetVertexVector();
        std::vector<int> sphereIndex = m_sphere->GetIndexVector();
        int sphereStart = stride * m_cylinderVector.size();

        int sphereStride = m_sphere->GetVertexBuffer()->GetCount();
        for(int i = 0; i < m_leafVector.size(); i++) {
            int start = sphereStride * i;
            // vertex positions
            auto matrix = m_leafVector[i];
            for (const auto& vertex : sphereVertex) {
                auto vertexAffine = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);
                auto normalAffine = glm::vec4(vertex.normal.x, vertex.normal.y, vertex.normal.z, 0);
                vertexAffine = matrix * vertexAffine;
                normalAffine = matrix * normalAffine;

                v.push_back(std::tuple<float, float, float>(vertexAffine.x, vertexAffine.y, vertexAffine.z));
                vt.push_back(std::tuple<float, float>(vertex.texCoord.x, vertex.texCoord.y));
                vn.push_back(std::tuple<float, float, float>(normalAffine.x, normalAffine.y, normalAffine.z));
            }

            // faces using indices
            size_t indexCount = sphereIndex.size();
            for (size_t j = 0; j < indexCount; j += 3) {
                uint32_t index1 = sphereIndex[j] + sphereStart + start + 1;
                uint32_t index2 = sphereIndex[j+1] + sphereStart + start + 1;
                uint32_t index3 = sphereIndex[j+2] + sphereStart + start + 1;

                f.push_back(std::tuple<int, int, int>(index1, index2, index3));
            }
        }
    }
    else {
        std::vector<Vertex> leafVertex = m_leaf->GetVertexVector();
        std::vector<int> leafIndex = m_leaf->GetIndexVector();
        int leafStart = stride * m_cylinderVector.size();
        
        int leafStride = m_leaf->GetVertexBuffer()->GetCount();
        for(int i = 0; i<m_leafVector.size(); i++) {
            int start = leafStride * i;
            // vertex positions
            auto matrix = m_leafVector[i];
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
                uint32_t index2 = leafIndex[j+1] + leafStart + start + 1;
                uint32_t index3 = leafIndex[j+2] + leafStart + start + 1;

                f.push_back(std::tuple<int, int, int>(index1, index2, index3));
            }
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
    out << "usemtl Tree\n";
    for(int i=0; i<f.size(); i++) {
        out << "f "
                << std::get<0>(f[i]) << "/" << std::get<0>(f[i]) << "/" << std::get<0>(f[i]) << " "
                << std::get<1>(f[i]) << "/" << std::get<1>(f[i]) << "/" << std::get<1>(f[i]) << " "
                << std::get<2>(f[i]) << "/" << std::get<2>(f[i]) << "/" << std::get<2>(f[i])
                << "\n";
    }

    return true;
}

bool LSystem::ExportMtl(std::ofstream& out, std::string texture) {
    if (!out.is_open()) {
        SPDLOG_ERROR("Failed to open file : {}", std::to_string(out.tellp()));
        return false;
    }

    out << "# Dice.mtl\n\n";

    out << "newmtl Tree\n\n";

    out << "# ambient color\n";
    out << "Ka 0.2 0.2 0.2\n\n";

    out << "# diffuse color\n";
    out << "Kd 0.7 0.7 0.7\n\n";

    out << "# specular color\n";
    out << "Ka 0.7 0.7 0.7\n\n";

    out << "# Blinn-Phong shading\n";
    out << "illum 2\n\n";

    out << "# texture\n";
    // out << "map_Kd ./" + texture + ".png\n";
    out << "map_Kd ./tree.png\n";

    return true;
}

bool LSystem::ExportTexture(const char* imageOutputPath) {
    m_treeImage->SaveImage(imageOutputPath);
    return true;
}