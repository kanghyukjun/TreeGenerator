#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "common.h"

CLASS_PTR(Buffer)
class Buffer {
public:
    /*
    VBO(정점 데이터를 담은 버퍼 오브젝트), 색상 위치 텍스쳐 법선벡터 등의 정보 포함
    VBO 생성 및 정점 데이터를 GPU로 복사
    
    GL_STATIC_DRAW : vertex의 위치를 세팅 후 바꾸지 않을 것임
    GL_DYNAMIC_DRAW : vertex의 위치를 바꿀 것임
    GL_STREAM_DRAW : 버퍼를 생성 후 한번 그린 다음에 버릴 것임
    */
    static BufferUPtr CreateWithData( uint32_t bufferType, uint32_t usage,
        const void* data, size_t stride, size_t count);

    ~Buffer();
    uint32_t Get() const { return m_buffer; }
    size_t GetStride() const { return m_stride; }
    size_t GetCount() const { return m_count; }
    void Bind() const;

private:
    Buffer() {}
    bool Init(uint32_t bufferType, uint32_t usage, const void* data, size_t stride, size_t count);
    uint32_t m_buffer { 0 };
    uint32_t m_bufferType { 0 };
    uint32_t m_usage { 0 };
    size_t m_stride { 0 };
    size_t m_count { 0 };
};

#endif // __BUFFER_H__