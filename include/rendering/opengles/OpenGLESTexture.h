#pragma once

#include "rendering/Texture.h"

namespace SmartRenderer {

class OpenGLESTexture : public Texture {
public:
    OpenGLESTexture(int width, int height, int format, const void* data);
    ~OpenGLESTexture() override;

    unsigned int GetID() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    void Bind(int unit = 0) override;
    void Unbind() override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace SmartRenderer
