#include "rendering/Buffer.h"

// OpenGL ES function declarations
typedef unsigned int GLuint;
typedef unsigned int GLenum;

extern "C" {
    void glGenBuffers(int n, GLuint* buffers);
    void glDeleteBuffers(int n, const GLuint* buffers);
    void glBindBuffer(GLenum target, GLuint buffer);
    void glBufferData(GLenum target, size_t size, const void* data, GLenum usage);
}

namespace SmartRenderer {

class OpenGLESBuffer : public Buffer {
public:
    OpenGLESBuffer(size_t size, const void* data, BufferUsage usage);
    ~OpenGLESBuffer() override;

    unsigned int GetID() const override { return m_bufferID; }
    void Bind() override;
    void Unbind() override;
    void SetData(size_t size, const void* data) override;

private:
    GLuint m_bufferID;
    size_t m_size;
    GLenum m_target;
    GLenum m_usage;
};

OpenGLESBuffer::OpenGLESBuffer(size_t size, const void* data, BufferUsage usage)
    : m_bufferID(0)
    , m_size(size) {
    
    m_target = 0x8892; // GL_ARRAY_BUFFER
    
    switch (usage) {
    case BufferUsage::Static:
        m_usage = 0x88E4; // GL_STATIC_DRAW
        break;
    case BufferUsage::Dynamic:
        m_usage = 0x88E8; // GL_DYNAMIC_DRAW
        break;
    case BufferUsage::Stream:
        m_usage = 0x88E0; // GL_STREAM_DRAW
        break;
    }
    
    glGenBuffers(1, &m_bufferID);
    glBindBuffer(m_target, m_bufferID);
    glBufferData(m_target, size, data, m_usage);
    glBindBuffer(m_target, 0);
}

OpenGLESBuffer::~OpenGLESBuffer() {
    if (m_bufferID != 0) {
        glDeleteBuffers(1, &m_bufferID);
        m_bufferID = 0;
    }
}

void OpenGLESBuffer::Bind() {
    glBindBuffer(m_target, m_bufferID);
}

void OpenGLESBuffer::Unbind() {
    glBindBuffer(m_target, 0);
}

void OpenGLESBuffer::SetData(size_t size, const void* data) {
    m_size = size;
    glBindBuffer(m_target, m_bufferID);
    glBufferData(m_target, size, data, m_usage);
    glBindBuffer(m_target, 0);
}

} // namespace SmartRenderer
