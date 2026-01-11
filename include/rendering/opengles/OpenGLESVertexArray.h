#pragma once

#include "rendering/VertexArray.h"

namespace SmartRenderer {

class OpenGLESVertexArray : public VertexArray {
public:
    OpenGLESVertexArray();
    ~OpenGLESVertexArray() override;

    unsigned int GetID() const override;
    void Bind() override;
    void Unbind() override;
    void SetVertexBuffer(Buffer* buffer, int index, int stride, int offset) override;
    void SetIndexBuffer(Buffer* buffer) override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace SmartRenderer
