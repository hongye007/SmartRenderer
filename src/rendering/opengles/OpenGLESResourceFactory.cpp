#include "rendering/opengles/OpenGLESResourceFactory.h"
#include "rendering/opengles/OpenGLESTypes.h"
#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include <memory>
#include <cstdio>

namespace SmartRenderer {

// Forward declarations of implementation classes
class OpenGLESShaderImpl : public Shader {
public:
    OpenGLESShaderImpl(const std::string& vertexSource, const std::string& fragmentSource);
    ~OpenGLESShaderImpl() override;
    bool IsValid() const override;
    unsigned int GetID() const override { return m_programID; }
    void Bind() override;
    void Unbind() override;
    void SetUniform(const std::string& name, float value) override;
    void SetUniform(const std::string& name, int value) override;
    void SetUniform(const std::string& name, float x, float y, float z, float w) override;
    void SetUniform(const std::string& name, const float* matrix4x4) override;

private:
    GLuint m_programID;
    bool m_valid;
    GLuint CompileShader(GLenum type, const std::string& source);
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};

class OpenGLESTextureImpl : public Texture {
public:
    OpenGLESTextureImpl(int width, int height, int format, const void* data);
    ~OpenGLESTextureImpl() override;
    unsigned int GetID() const override { return m_textureID; }
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    void Bind(int unit = 0) override;
    void Unbind() override;

private:
    GLuint m_textureID;
    int m_width;
    int m_height;
};

class OpenGLESBufferImpl : public Buffer {
public:
    OpenGLESBufferImpl(size_t size, const void* data, BufferUsage usage);
    ~OpenGLESBufferImpl() override;
    unsigned int GetID() const override { return m_bufferID; }
    void Bind() override;
    void Unbind() override;
    void SetData(size_t size, const void* data) override;

private:
    GLuint m_bufferID;
    GLenum m_target;
    GLenum m_usage;
};

class OpenGLESVertexArrayImpl : public VertexArray {
public:
    OpenGLESVertexArrayImpl();
    ~OpenGLESVertexArrayImpl() override;
    unsigned int GetID() const override { return m_vaoID; }
    void Bind() override;
    void Unbind() override;
    void SetVertexBuffer(Buffer* buffer, int index, int size, int stride, int offset) override;
    void SetIndexBuffer(Buffer* buffer) override;

private:
    GLuint m_vaoID;
    Buffer* m_indexBuffer;
};

class OpenGLESFramebufferImpl : public Framebuffer {
public:
    OpenGLESFramebufferImpl(int width, int height);
    ~OpenGLESFramebufferImpl() override;
    unsigned int GetID() const override { return m_fboID; }
    void Bind() override;
    void Unbind() override;
    Texture* GetColorTexture() override;
    Texture* GetDepthTexture() override { return nullptr; }

private:
    GLuint m_fboID;
    GLuint m_colorTextureID;
    GLuint m_depthRenderbuffer;
    int m_width;
    int m_height;
    std::unique_ptr<Texture> m_colorTexture;
};

// Shader Implementation
OpenGLESShaderImpl::OpenGLESShaderImpl(const std::string& vertexSource, const std::string& fragmentSource)
    : m_programID(0), m_valid(false) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) return;

    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return;
    }

    m_valid = LinkProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

OpenGLESShaderImpl::~OpenGLESShaderImpl() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

GLuint OpenGLESShaderImpl::CompileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        #ifdef _DEBUG
        fprintf(stderr, "Failed to create shader (type: %d)\n", type);
        #endif
        return 0;
    }

    const char* sourcePtr = source.c_str();
    GLint length = static_cast<GLint>(source.length());
    glShaderSource(shader, 1, &sourcePtr, &length);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        GLint infoLogLength = 0;
        glGetShaderInfoLog(shader, 512, &infoLogLength, infoLog);
        fprintf(stderr, "Shader compilation failed (%s):\n%s\n", 
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool OpenGLESShaderImpl::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    m_programID = glCreateProgram();
    if (m_programID == 0) {
        fprintf(stderr, "Failed to create shader program\n");
        return false;
    }

    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    
    // Bind attribute location for GLSL ES 100 (OpenGL ES 2.0)
    // In ES 2.0, we need to bind attributes before linking
    glBindAttribLocation(m_programID, 0, "aPosition");
    glBindAttribLocation(m_programID, 1, "aTexCoord");
    
    glLinkProgram(m_programID);

    GLint success = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        GLint infoLogLength = 0;
        glGetProgramInfoLog(m_programID, 512, &infoLogLength, infoLog);
        fprintf(stderr, "Shader program linking failed:\n%s\n", infoLog);
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }
    return true;
}

bool OpenGLESShaderImpl::IsValid() const {
    return m_programID != 0 && m_valid;
}

void OpenGLESShaderImpl::Bind() {
    if (m_programID != 0) glUseProgram(m_programID);
}

void OpenGLESShaderImpl::Unbind() {
    glUseProgram(0);
}

void OpenGLESShaderImpl::SetUniform(const std::string& name, float value) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) glUniform1f(location, value);
    }
}

void OpenGLESShaderImpl::SetUniform(const std::string& name, int value) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) glUniform1i(location, value);
    }
}

void OpenGLESShaderImpl::SetUniform(const std::string& name, float x, float y, float z, float w) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) {
            glUniform4f(location, x, y, z, w);
        }
    }
}

void OpenGLESShaderImpl::SetUniform(const std::string& name, const float* matrix4x4) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) glUniformMatrix4fv(location, 1, 0 /* GL_FALSE */, matrix4x4);
    }
}

// Texture Implementation
OpenGLESTextureImpl::OpenGLESTextureImpl(int width, int height, int format, const void* data)
    : m_textureID(0), m_width(width), m_height(height) {
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    GLenum glFormat = (format == 1) ? GL_RGB : GL_RGBA;
    int internalFormat = (format == 1) ? GL_RGB8 : GL_RGBA8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLESTextureImpl::~OpenGLESTextureImpl() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

void OpenGLESTextureImpl::Bind(int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void OpenGLESTextureImpl::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Buffer Implementation
OpenGLESBufferImpl::OpenGLESBufferImpl(size_t size, const void* data, BufferUsage usage)
    : m_bufferID(0), m_target(GL_ARRAY_BUFFER) {
    switch (usage) {
    case BufferUsage::Static: m_usage = GL_STATIC_DRAW; break;
    case BufferUsage::Dynamic: m_usage = GL_DYNAMIC_DRAW; break;
    case BufferUsage::Stream: m_usage = GL_STREAM_DRAW; break;
    }

    glGenBuffers(1, &m_bufferID);
    glBindBuffer(m_target, m_bufferID);
    glBufferData(m_target, static_cast<GLsizeiptr>(size), data, m_usage);
    glBindBuffer(m_target, 0);
}

OpenGLESBufferImpl::~OpenGLESBufferImpl() {
    if (m_bufferID != 0) {
        glDeleteBuffers(1, &m_bufferID);
    }
}

void OpenGLESBufferImpl::Bind() {
    glBindBuffer(m_target, m_bufferID);
}

void OpenGLESBufferImpl::Unbind() {
    glBindBuffer(m_target, 0);
}

void OpenGLESBufferImpl::SetData(size_t size, const void* data) {
    glBindBuffer(m_target, m_bufferID);
    glBufferData(m_target, static_cast<GLsizeiptr>(size), data, m_usage);
    glBindBuffer(m_target, 0);
}

// VertexArray Implementation
OpenGLESVertexArrayImpl::OpenGLESVertexArrayImpl()
    : m_vaoID(0), m_indexBuffer(nullptr) {
    glGenVertexArrays(1, &m_vaoID);
}

OpenGLESVertexArrayImpl::~OpenGLESVertexArrayImpl() {
    if (m_vaoID != 0) {
        glDeleteVertexArrays(1, &m_vaoID);
    }
}

void OpenGLESVertexArrayImpl::Bind() {
    glBindVertexArray(m_vaoID);
}

void OpenGLESVertexArrayImpl::Unbind() {
    glBindVertexArray(0);
}

void OpenGLESVertexArrayImpl::SetVertexBuffer(Buffer* buffer, int index, int size, int stride, int offset) {
    if (index >= 0 && index < 16 && buffer) {
        Bind();
        buffer->Bind();
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, GL_FLOAT, 0, stride, reinterpret_cast<const void*>(static_cast<intptr_t>(offset)));
        Unbind();
    }
}

void OpenGLESVertexArrayImpl::SetIndexBuffer(Buffer* buffer) {
    m_indexBuffer = buffer;
    Bind();
    if (buffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->GetID());
    }
    Unbind();
}

// Framebuffer Implementation
OpenGLESFramebufferImpl::OpenGLESFramebufferImpl(int width, int height)
    : m_fboID(0), m_colorTextureID(0), m_depthRenderbuffer(0), m_width(width), m_height(height) {
    glGenFramebuffers(1, &m_fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

    // Create color texture
    glGenTextures(1, &m_colorTextureID);
    glBindTexture(GL_TEXTURE_2D, m_colorTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextureID, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLESFramebufferImpl::~OpenGLESFramebufferImpl() {
    if (m_depthRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
    }
    if (m_colorTextureID != 0) {
        glDeleteTextures(1, &m_colorTextureID);
    }
    if (m_fboID != 0) {
        glDeleteFramebuffers(1, &m_fboID);
    }
}

void OpenGLESFramebufferImpl::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
}

void OpenGLESFramebufferImpl::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Texture* OpenGLESFramebufferImpl::GetColorTexture() {
    // Return a wrapper texture for the color attachment
    // In a full implementation, would create a proper Texture wrapper around m_colorTextureID
    return nullptr; // Simplified - would need proper texture wrapper implementation
}

// Factory functions
Shader* CreateOpenGLESShader(const std::string& vertexSource, const std::string& fragmentSource) {
    return new OpenGLESShaderImpl(vertexSource, fragmentSource);
}

Texture* CreateOpenGLESTexture(int width, int height, int format, const void* data) {
    return new OpenGLESTextureImpl(width, height, format, data);
}

Buffer* CreateOpenGLESBuffer(size_t size, const void* data, BufferUsage usage) {
    return new OpenGLESBufferImpl(size, data, usage);
}

VertexArray* CreateOpenGLESVertexArray() {
    return new OpenGLESVertexArrayImpl();
}

Framebuffer* CreateOpenGLESFramebuffer(int width, int height) {
    return new OpenGLESFramebufferImpl(width, height);
}

} // namespace SmartRenderer
