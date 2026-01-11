#include "rendering/Framebuffer.h"
#include "rendering/Texture.h"

// OpenGL ES function declarations
typedef unsigned int GLuint;
typedef unsigned int GLenum;

extern "C" {
    void glGenFramebuffers(int n, GLuint* framebuffers);
    void glDeleteFramebuffers(int n, const GLuint* framebuffers);
    void glBindFramebuffer(GLenum target, GLuint framebuffer);
    void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, int level);
    void glGenRenderbuffers(int n, GLuint* renderbuffers);
    void glDeleteRenderbuffers(int n, const GLuint* renderbuffers);
    void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
    void glRenderbufferStorage(GLenum target, GLenum internalformat, int width, int height);
    void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
}

namespace SmartRenderer {

class OpenGLESFramebuffer : public Framebuffer {
public:
    OpenGLESFramebuffer(int width, int height);
    ~OpenGLESFramebuffer() override;

    unsigned int GetID() const override { return m_fboID; }
    void Bind() override;
    void Unbind() override;
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }

private:
    GLuint m_fboID;
    GLuint m_depthRenderbuffer;
    std::unique_ptr<Texture> m_colorTexture;
    std::unique_ptr<Texture> m_depthTexture;
    int m_width;
    int m_height;
};

OpenGLESFramebuffer::OpenGLESFramebuffer(int width, int height)
    : m_fboID(0)
    , m_depthRenderbuffer(0)
    , m_width(width)
    , m_height(height) {
    
    glGenFramebuffers(1, &m_fboID);
    glBindFramebuffer(0x8D40 /* GL_FRAMEBUFFER */, m_fboID);
    
    // Create color texture
    m_colorTexture = std::make_unique<Texture>(); // Would create actual texture
    // glFramebufferTexture2D(0x8D40, 0x8CE0, 0x0DE1, m_colorTexture->GetID(), 0);
    
    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(0x8D41 /* GL_RENDERBUFFER */, m_depthRenderbuffer);
    glRenderbufferStorage(0x8D41, 0x81A5 /* GL_DEPTH_COMPONENT24 */, width, height);
    glFramebufferRenderbuffer(0x8D40, 0x8D00 /* GL_DEPTH_ATTACHMENT */, 0x8D41, m_depthRenderbuffer);
    
    glBindFramebuffer(0x8D40, 0);
}

OpenGLESFramebuffer::~OpenGLESFramebuffer() {
    if (m_depthRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }
    if (m_fboID != 0) {
        glDeleteFramebuffers(1, &m_fboID);
        m_fboID = 0;
    }
}

void OpenGLESFramebuffer::Bind() {
    glBindFramebuffer(0x8D40 /* GL_FRAMEBUFFER */, m_fboID);
}

void OpenGLESFramebuffer::Unbind() {
    glBindFramebuffer(0x8D40 /* GL_FRAMEBUFFER */, 0);
}

} // namespace SmartRenderer
