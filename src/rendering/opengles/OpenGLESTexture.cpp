#include "rendering/Texture.h"
#include <cstring>

// OpenGL ES function declarations
typedef unsigned int GLuint;
typedef unsigned int GLenum;

extern "C" {
    void glGenTextures(int n, GLuint* textures);
    void glDeleteTextures(int n, const GLuint* textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexImage2D(GLenum target, int level, int internalformat, int width, int height, int border, GLenum format, GLenum type, const void* data);
    void glTexParameteri(GLenum target, GLenum pname, int param);
}

namespace SmartRenderer {

class OpenGLESTexture : public Texture {
public:
    OpenGLESTexture(int width, int height, int format, const void* data);
    ~OpenGLESTexture() override;

    unsigned int GetID() const override { return m_textureID; }
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    void Bind(int unit = 0) override;
    void Unbind() override;

private:
    GLuint m_textureID;
    int m_width;
    int m_height;
    int m_format;
};

OpenGLESTexture::OpenGLESTexture(int width, int height, int format, const void* data)
    : m_textureID(0)
    , m_width(width)
    , m_height(height)
    , m_format(format) {
    
    glGenTextures(1, &m_textureID);
    glBindTexture(0x0DE1 /* GL_TEXTURE_2D */, m_textureID);
    
    // Determine format
    GLenum glFormat = 0x1908; // GL_RGBA
    GLenum glType = 0x1401; // GL_UNSIGNED_BYTE
    int internalFormat = 0x8058; // GL_RGBA8
    
    if (format == 1) { // RGB
        glFormat = 0x1907; // GL_RGB
        internalFormat = 0x8051; // GL_RGB8
    }
    
    glTexImage2D(0x0DE1 /* GL_TEXTURE_2D */, 0, internalFormat, width, height, 0, glFormat, glType, data);
    
    // Set default parameters
    glTexParameteri(0x0DE1 /* GL_TEXTURE_2D */, 0x2801 /* GL_TEXTURE_MIN_FILTER */, 0x2601 /* GL_LINEAR */);
    glTexParameteri(0x0DE1 /* GL_TEXTURE_2D */, 0x2800 /* GL_TEXTURE_MAG_FILTER */, 0x2601 /* GL_LINEAR */);
    glTexParameteri(0x0DE1 /* GL_TEXTURE_2D */, 0x2802 /* GL_TEXTURE_WRAP_S */, 0x2901 /* GL_REPEAT */);
    glTexParameteri(0x0DE1 /* GL_TEXTURE_2D */, 0x2803 /* GL_TEXTURE_WRAP_T */, 0x2901 /* GL_REPEAT */);
    
    glBindTexture(0x0DE1 /* GL_TEXTURE_2D */, 0);
}

OpenGLESTexture::~OpenGLESTexture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

void OpenGLESTexture::Bind(int unit) {
    // glActiveTexture(0x84C0 + unit); // GL_TEXTURE0 + unit
    glBindTexture(0x0DE1 /* GL_TEXTURE_2D */, m_textureID);
}

void OpenGLESTexture::Unbind() {
    glBindTexture(0x0DE1 /* GL_TEXTURE_2D */, 0);
}

} // namespace SmartRenderer
