#include "rendering/VertexArray.h"
#include "rendering/Buffer.h"

// OpenGL ES function declarations
typedef unsigned int GLuint;
typedef unsigned int GLenum;

extern "C" {
    void glGenVertexArrays(int n, GLuint* arrays);
    void glDeleteVertexArrays(int n, const GLuint* arrays);
    void glBindVertexArray(GLuint array);
    void glEnableVertexAttribArray(unsigned int index);
    void glVertexAttribPointer(unsigned int index, int size, GLenum type, unsigned char normalized, int stride, const void* pointer);
    void glBindBuffer(GLenum target, GLuint buffer);
}

namespace SmartRenderer {

class OpenGLESVertexArray : public VertexArray {
public:
    OpenGLESVertexArray();
    ~OpenGLESVertexArray() override;

    unsigned int GetID() const override { return m_vaoID; }
    void Bind() override;
    void Unbind() override;
    void SetVertexBuffer(Buffer* buffer, int index, int stride, int offset) override;
    void SetIndexBuffer(Buffer* buffer) override;

private:
    GLuint m_vaoID;
    Buffer* m_vertexBuffers[16];
    Buffer* m_indexBuffer;
};

OpenGLESVertexArray::OpenGLESVertexArray()
    : m_vaoID(0)
    , m_indexBuffer(nullptr) {
    for (int i = 0; i < 16; ++i) {
        m_vertexBuffers[i] = nullptr;
    }
    
    glGenVertexArrays(1, &m_vaoID);
}

OpenGLESVertexArray::~OpenGLESVertexArray() {
    if (m_vaoID != 0) {
        glDeleteVertexArrays(1, &m_vaoID);
        m_vaoID = 0;
    }
}

void OpenGLESVertexArray::Bind() {
    glBindVertexArray(m_vaoID);
}

void OpenGLESVertexArray::Unbind() {
    glBindVertexArray(0);
}

void OpenGLESVertexArray::SetVertexBuffer(Buffer* buffer, int index, int stride, int offset) {
    if (index >= 0 && index < 16) {
        m_vertexBuffers[index] = buffer;
        Bind();
        if (buffer) {
            buffer->Bind();
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, 3, 0x1406 /* GL_FLOAT */, 0, stride, reinterpret_cast<const void*>(offset));
        }
        Unbind();
    }
}

void OpenGLESVertexArray::SetIndexBuffer(Buffer* buffer) {
    m_indexBuffer = buffer;
    Bind();
    if (buffer) {
        glBindBuffer(0x8893 /* GL_ELEMENT_ARRAY_BUFFER */, buffer->GetID());
    }
    Unbind();
}

} // namespace SmartRenderer
