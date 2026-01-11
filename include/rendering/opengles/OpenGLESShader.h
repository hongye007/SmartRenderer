#pragma once

#include "rendering/Shader.h"
#include <string>

namespace SmartRenderer {

// Forward declaration
class OpenGLESShaderImpl;

class OpenGLESShader : public Shader {
public:
    OpenGLESShader(const std::string& vertexSource, const std::string& fragmentSource);
    ~OpenGLESShader() override;

    bool IsValid() const override;
    unsigned int GetID() const override;
    void Bind() override;
    void Unbind() override;
    void SetUniform(const std::string& name, float value) override;
    void SetUniform(const std::string& name, int value) override;

private:
    std::unique_ptr<OpenGLESShaderImpl> m_impl;
};

} // namespace SmartRenderer
