#ifndef __VERTEX_LAYOUT_H__
#define __VERTEX_LAYOUT_H__

#include "common.h"

CLASS_PTR(VertexLayout)
class VertexLayout {
public:
    static VertexLayoutUPtr Create();
    ~VertexLayout();

    uint32_t Get() const { return m_vertexArrayObject; }
    void Bind() const;
    
    /*
    attribute의 생김새 지정
    정점의 n번째 attribute, point가 3개, 각각은 float 값,
    normalize 필요 없음, stride의 크기, 첫 정점의 해당 attribute 까지의 간격(offset)
    */
    void SetAttrib(uint32_t attribIndex, int count,
                    uint32_t type, bool normalized,
                    size_t stride, uint64_t offset) const;
    void DisableAttrib(int attribIndex) const;

private:
    VertexLayout() {}
    void Init();
    uint32_t m_vertexArrayObject { 0 };
};

#endif // __VERTEX_LAYOUT_H__