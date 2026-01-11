#pragma once

#include "rendering/Buffer.h"

namespace SmartRenderer {

class OpenGLESBuffer : public Buffer {
public:
    OpenGLESBuffer(size_t size, const void* data, BufferUsage usage);
    ~OpenGLESBuffer() override;

    unsigned int GetID() const override;
    void Bind() override;
    void Unbind() override;
    void SetData(size_t size, const void* data) override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace SmartRenderer
