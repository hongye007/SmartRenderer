#pragma once

#include <string>
#include <memory>

namespace SmartRenderer {

class Shader {
public:
    virtual ~Shader() = default;
    virtual bool IsValid() const = 0;
    virtual unsigned int GetID() const = 0;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void SetUniform(const std::string& name, float value) = 0;
    virtual void SetUniform(const std::string& name, int value) = 0;
    virtual void SetUniform(const std::string& name, float x, float y, float z, float w) = 0;
    virtual void SetUniform(const std::string& name, const float* matrix4x4) = 0;
};

} // namespace SmartRenderer
