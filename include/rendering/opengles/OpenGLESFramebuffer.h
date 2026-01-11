#pragma once

#include "rendering/Framebuffer.h"

namespace SmartRenderer {

class OpenGLESFramebuffer : public Framebuffer {
public:
    OpenGLESFramebuffer(int width, int height);
    ~OpenGLESFramebuffer() override;

    unsigned int GetID() const override;
    void Bind() override;
    void Unbind() override;
    Texture* GetColorTexture() override;
    Texture* GetDepthTexture() override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace SmartRenderer
