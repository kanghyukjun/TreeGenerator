#include "mesh.h"
#define _USE_MATH_DEFINES
#include <math.h>

MeshUPtr Mesh::Create(const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices, uint32_t primitiveType) {

    auto mesh = MeshUPtr(new Mesh());
    mesh->Init(vertices, indices, primitiveType);
    return std::move(mesh);
}

void Mesh::Init(const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices, uint32_t primitiveType) {

    if (primitiveType == GL_TRIANGLES) {
        ComputeTangents(const_cast<std::vector<Vertex>&>(vertices), indices);
    }

    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
        vertices.data(), sizeof(Vertex), vertices.size());
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
        indices.data(), sizeof(uint32_t), indices.size());
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, false, sizeof(Vertex), 0); // position
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, normal)); // normal
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, texCoord)); // tex
    m_vertexLayout->SetAttrib(3, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, tangent)); // tex

    m_vertexVector.assign(vertices.begin(), vertices.end());
    m_indexVector.assign(indices.begin(), indices.end());
}

void Mesh::Draw(const Program* program) const {
    m_vertexLayout->Bind();
    if (m_material) {
        m_material->SetToProgram(program);
    }

    glDrawElements(m_primitiveType, m_indexBuffer->GetCount(), GL_UNSIGNED_INT, 0);
}

MeshUPtr Mesh::CreateBox() {
    std::vector<Vertex> vertices = {
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },

        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    };

    std::vector<uint32_t> indices = {
        0,  2,  1,  2,  0,  3,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 14, 13, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 22, 20, 23,
    };

    return Create(vertices, indices, GL_TRIANGLES);
}

MeshUPtr Mesh::CreatePlane() {
    std::vector<Vertex> vertices = {
        Vertex { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    };

    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,
    };

    return Create(vertices, indices, GL_TRIANGLES);
}

MeshUPtr Mesh::CreateCylinder(const float radius, const float height, const float rate){
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    const int numSlices = 50;

    float textureRadius = 0.218f;
    float textureIncrement = 0.958f / static_cast<float>(numSlices);

    // Create the top cap vertices.
    glm::vec3 topCenter = glm::vec3(0.0f, height / 2.0f, 0.0f);
    vertices.push_back(Vertex{topCenter, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.717f, 0.740f), glm::vec3(0.0f, 0.0f, 0.0f)});
    float angleIncrement = glm::two_pi<float>() / numSlices;
    float topRadius = radius * rate;
    for (int i = 0; i < numSlices; i++) {
        float angle = angleIncrement * i;
        glm::vec3 pos = glm::vec3(glm::cos(angle) * topRadius, height / 2.0f, glm::sin(angle) * topRadius);
        vertices.push_back(Vertex{pos, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec2(0.717f + textureRadius * glm::cos(angle), 0.740f - textureRadius * glm::sin(angle)), glm::vec3(0.0f, 0.0f, 0.0f)});
    }

    // Create the bottom cap vertices.
    glm::vec3 bottomCenter = glm::vec3(0.0f, -height / 2.0f, 0.0f);
    vertices.push_back(Vertex{bottomCenter, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.717f, 0.740f), glm::vec3(0.0f, 0.0f, 0.0f)});
    for (int i = 0; i < numSlices; i++) {
        float angle = angleIncrement * i;
        glm::vec3 pos = glm::vec3(glm::cos(angle) * radius, -height / 2.0f, glm::sin(angle) * radius);
        vertices.push_back(Vertex{pos, glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec2(0.717f + textureRadius * glm::cos(angle), 0.740f - textureRadius * glm::sin(angle)), glm::vec3(0.0f, 0.0f, 0.0f)});
    }

    // Create the side vertices.
    for (int i = 0; i < numSlices; i++) {
        float angle = angleIncrement * i;
        float increment = textureIncrement * i;
        glm::vec3 posTop = glm::vec3(glm::cos(angle) * topRadius, height / 2.0f, glm::sin(angle) * topRadius);
        vertices.push_back(Vertex{posTop, glm::normalize(glm::vec3(glm::cos(angle) * radius, radius / height * (radius - topRadius),
            glm::sin(angle) * radius)), glm::vec2(0.976f - increment, 0.474f), glm::vec3(0.0f, 0.0f, 0.0f)});
    }
    for (int i = 0; i < numSlices; i++) {
        float angle = angleIncrement * i;
        float increment = textureIncrement * i;
        glm::vec3 posBottom = glm::vec3(glm::cos(angle) * radius, -height / 2.0f, glm::sin(angle) * radius);
        vertices.push_back(Vertex{posBottom, glm::normalize(glm::vec3(glm::cos(angle) * radius, radius / height * (radius - topRadius),
            glm::sin(angle) * radius)), glm::vec2(0.976f - increment, 0.118f), glm::vec3(0.0f, 0.0f, 0.0f)});
    }

    // Create the top cap indices.
    for (int i = 1; i < numSlices; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }
    indices.push_back(0);
    indices.push_back(numSlices);
    indices.push_back(1);

    // Create the bottom cap indices.
    int bottomCenterIndex = numSlices + 1;
    for (int i = bottomCenterIndex + 1; i < bottomCenterIndex + numSlices; i++) {
        indices.push_back(bottomCenterIndex);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    indices.push_back(bottomCenterIndex);
    indices.push_back(numSlices * 2 + 1);
    indices.push_back(bottomCenterIndex + 1);

    // Create the side indices.
    for (int i = numSlices * 2 + 2; i < numSlices * 3 + 1; i++){
        indices.push_back(i + numSlices + 1);
        indices.push_back(i + numSlices);
        indices.push_back(i);
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + numSlices + 1);
    }
    indices.push_back(numSlices * 3 + 2);
    indices.push_back(numSlices * 4 + 1);
    indices.push_back(numSlices * 3 + 1);
    indices.push_back(numSlices * 3 + 1);
    indices.push_back(numSlices * 2 + 2);
    indices.push_back(numSlices * 3 + 2);

    return Create(vertices, indices, GL_TRIANGLES);
}

MeshUPtr Mesh::CreateLeaf(float width, float height) {
    std::vector<Vertex> vertices = {
        Vertex { glm::vec3( 0.0f,  0.0f, width / 2.0f), glm::vec3( 1.0f,  0.0f, 0.0f), glm::vec2(0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.0f,  0.0f, width / -2.0f), glm::vec3( 1.0f,  0.0f, 0.0f), glm::vec2(0.47f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.0f,  height, width / -2.0f), glm::vec3( 1.0f,  0.0f, 0.0f), glm::vec2(0.47f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.0f,  height, width / 2.0f), glm::vec3( 1.0f,  0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },

        Vertex { glm::vec3( 0.0f,  0.0f, width / 2.0f), glm::vec3( -1.0f,  0.0f, 0.0f), glm::vec2(0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.0f,  0.0f, width / -2.0f), glm::vec3( -1.0f,  0.0f, 0.0f), glm::vec2(0.47f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.0f,  height, width / -2.0f), glm::vec3( -1.0f,  0.0f, 0.0f), glm::vec2(0.47f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
        Vertex { glm::vec3( 0.0f,  height, width / 2.0f), glm::vec3( -1.0f,  0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 7, 5, 5, 7, 6
    };

    return Create(vertices, indices, GL_TRIANGLES);
}

MeshUPtr Mesh::CreateSphere(float radius) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const int stacks = 15;
    const int slices = 30;
    float theta, phi;

    for(int i = 0; i <= stacks; i++) {
        phi = M_PI / 2 - i * M_PI / stacks;
        for(int j = 0; j <= slices; j++) {
            theta = j * 2 * M_PI / slices;

            float xn = cos(phi) * sin(theta);
            float yn = sin(phi);
            float zn = cos(phi) * cos(theta);

            vertices.push_back(Vertex { glm::vec3(radius * xn + radius / 2, radius * yn + radius / 2, radius * zn + radius / 2),
                glm::vec3(xn, yn, zn), glm::vec2(0.0f), glm::vec3(0.0f)});
        }
    }

    for(int i = 0; i < stacks; i++) {
        for(int j = 0; j < slices; j++) {
            int currRow = i * (slices + 1); // slices 총 slices + 1개씩 생성
            int nextRow = (i + 1) * (slices + 1);

            indices.push_back(currRow + j);
            indices.push_back(nextRow + j);
            indices.push_back(nextRow + j + 1);

            indices.push_back(nextRow + j + 1);
            indices.push_back(currRow + j + 1);
            indices.push_back(currRow + j);
        }
    }

    return Create(vertices, indices, GL_TRIANGLES);
}

void Material::SetToProgram(const Program* program) const {
    int textureCount = 0;
    if (diffuse) {
        glActiveTexture(GL_TEXTURE0 + textureCount);
        program->SetUniform("material.diffuse", textureCount);
        diffuse->Bind();
        textureCount++;
    }
    if (specular) {
        glActiveTexture(GL_TEXTURE0 + textureCount);
        program->SetUniform("material.specular", textureCount);
        specular->Bind();
        textureCount++;
    }
    glActiveTexture(GL_TEXTURE0);
    program->SetUniform("material.shininess", shininess);
}

void Mesh::ComputeTangents(std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices) {

    auto compute = [](
        const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3,
        const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3)
        -> glm::vec3 {

        auto edge1 = pos2 - pos1;
        auto edge2 = pos3 - pos1;
        auto deltaUV1 = uv2 - uv1;
        auto deltaUV2 = uv3 - uv1;
        float det = (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        if (det != 0.0f) {
            auto invDet = 1.0f / det;
            return invDet * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        }
        else {
            return glm::vec3(0.0f, 0.0f, 0.0f);
        }
    };

    // initialize
    std::vector<glm::vec3> tangents;
    tangents.resize(vertices.size());
    memset(tangents.data(), 0, tangents.size() * sizeof(glm::vec3));

    // accumulate triangle tangents to each vertex
    for (size_t i = 0; i < indices.size(); i += 3) {
        auto v1 = indices[i  ];
        auto v2 = indices[i+1];
        auto v3 = indices[i+2];

        tangents[v1] += compute(
            vertices[v1].position, vertices[v2].position, vertices[v3].position,
            vertices[v1].texCoord, vertices[v2].texCoord, vertices[v3].texCoord);

        tangents[v2] = compute(
            vertices[v2].position, vertices[v3].position, vertices[v1].position,
            vertices[v2].texCoord, vertices[v3].texCoord, vertices[v1].texCoord);

        tangents[v3] = compute(
            vertices[v3].position, vertices[v1].position, vertices[v2].position,
            vertices[v3].texCoord, vertices[v1].texCoord, vertices[v2].texCoord);
    }

    // normalize
    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i].tangent = glm::normalize(tangents[i]);
    }
}