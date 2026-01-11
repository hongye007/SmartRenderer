#pragma once

#include <string>

namespace SmartRenderer {

class RenderResource {
public:
    enum class Type {
        Texture,
        Buffer,
        Framebuffer
    };

    virtual ~RenderResource() = default;
    virtual Type GetType() const = 0;
    virtual std::string GetName() const = 0;
};

} // namespace SmartRenderer
