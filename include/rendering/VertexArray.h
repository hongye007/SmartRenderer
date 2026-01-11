#pragma once

namespace SmartRenderer {

class Buffer;

class VertexArray {
public:
    virtual ~VertexArray() = default;
    virtual unsigned int GetID() const = 0;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void SetVertexBuffer(Buffer* buffer, int index, int size, int stride, int offset) = 0;
    virtual void SetIndexBuffer(Buffer* buffer) = 0;
};

} // namespace SmartRenderer
