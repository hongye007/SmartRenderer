#pragma once

namespace SmartRenderer {

class Texture;

class Framebuffer {
public:
    virtual ~Framebuffer() = default;
    virtual unsigned int GetID() const = 0;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual Texture* GetColorTexture() = 0;
    virtual Texture* GetDepthTexture() = 0;
};

} // namespace SmartRenderer
