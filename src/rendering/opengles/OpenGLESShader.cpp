#include "rendering/Shader.h"
#include <string>
#include <vector>
#include <stdexcept>

// OpenGL ES function declarations (would normally come from ANGLE headers)
typedef unsigned int GLuint;
typedef int GLint;
typedef char GLchar;

extern "C" {
    GLuint glCreateShader(unsigned int type);
    void glShaderSource(GLuint shader, int count, const GLchar* const* string, const int* length);
    void glCompileShader(GLuint shader);
    void glGetShaderiv(GLuint shader, unsigned int pname, int* params);
    void glGetShaderInfoLog(GLuint shader, int bufSize, int* length, char* infoLog);
    void glDeleteShader(GLuint shader);
    GLuint glCreateProgram();
    void glAttachShader(GLuint program, GLuint shader);
    void glLinkProgram(GLuint program);
    void glGetProgramiv(GLuint program, unsigned int pname, int* params);
    void glGetProgramInfoLog(GLuint program, int bufSize, int* length, char* infoLog);
    void glDeleteProgram(GLuint program);
    void glUseProgram(GLuint program);
    GLint glGetUniformLocation(GLuint program, const char* name);
    void glUniform1f(GLint location, float v0);
    void glUniform1i(GLint location, int v0);
    void glUniform4f(GLint location, float v0, float v1, float v2, float v3);
    void glUniformMatrix4fv(GLint location, int count, unsigned char transpose, const float* value);
}

namespace SmartRenderer {

class OpenGLESShader : public Shader {
public:
    OpenGLESShader(const std::string& vertexSource, const std::string& fragmentSource);
    ~OpenGLESShader() override;

    bool IsValid() const override { return m_programID != 0 && m_valid; }
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
    
    GLuint CompileShader(unsigned int type, const std::string& source);
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};

OpenGLESShader::OpenGLESShader(const std::string& vertexSource, const std::string& fragmentSource)
    : m_programID(0)
    , m_valid(false) {
    
    GLuint vertexShader = CompileShader(0x8B31 /* GL_VERTEX_SHADER */, vertexSource);
    if (vertexShader == 0) {
        return;
    }

    GLuint fragmentShader = CompileShader(0x8B30 /* GL_FRAGMENT_SHADER */, fragmentSource);
    if (fragmentShader == 0) {
        // glDeleteShader(vertexShader);
        return;
    }

    m_valid = LinkProgram(vertexShader, fragmentShader);
    
    // Clean up shaders
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);
}

OpenGLESShader::~OpenGLESShader() {
    if (m_programID != 0) {
        // glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

GLuint OpenGLESShader::CompileShader(unsigned int type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        return 0;
    }

    const char* sourcePtr = source.c_str();
    int length = static_cast<int>(source.length());
    glShaderSource(shader, 1, &sourcePtr, &length);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, 0x8B81 /* GL_COMPILE_STATUS */, &success);
    if (!success) {
        char infoLog[512];
        int infoLogLength = 0;
        glGetShaderInfoLog(shader, 512, &infoLogLength, infoLog);
        // Log error
        // glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool OpenGLESShader::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    m_programID = glCreateProgram();
    if (m_programID == 0) {
        return false;
    }

    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);

    int success = 0;
    glGetProgramiv(m_programID, 0x8B82 /* GL_LINK_STATUS */, &success);
    if (!success) {
        char infoLog[512];
        int infoLogLength = 0;
        glGetProgramInfoLog(m_programID, 512, &infoLogLength, infoLog);
        // Log error
        // glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    return true;
}

void OpenGLESShader::Bind() {
    if (m_programID != 0) {
        glUseProgram(m_programID);
    }
}

void OpenGLESShader::Unbind() {
    glUseProgram(0);
}

void OpenGLESShader::SetUniform(const std::string& name, float value) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) {
            glUniform1f(location, value);
        }
    }
}

void OpenGLESShader::SetUniform(const std::string& name, int value) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) {
            glUniform1i(location, value);
        }
    }
}

void OpenGLESShader::SetUniform(const std::string& name, float x, float y, float z, float w) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) {
            glUniform4f(location, x, y, z, w);
        }
    }
}

void OpenGLESShader::SetUniform(const std::string& name, const float* matrix4x4) {
    if (m_programID != 0) {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location >= 0) {
            glUniformMatrix4fv(location, 1, 0 /* GL_FALSE */, matrix4x4);
        }
    }
}

} // namespace SmartRenderer
