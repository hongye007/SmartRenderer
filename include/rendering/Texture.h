#pragma once

#include <cstdint>

namespace SmartRenderer {

class Texture {
public:
    virtual ~Texture() = default;
    virtual unsigned int GetID() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual void Bind(int unit = 0) = 0;
    virtual void Unbind() = 0;
};

} // namespace SmartRenderer
