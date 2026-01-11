#pragma once

#include <cstddef>

namespace SmartRenderer {

enum class BufferUsage {
    Static,
    Dynamic,
    Stream
};

class Buffer {
public:
    virtual ~Buffer() = default;
    virtual unsigned int GetID() const = 0;
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void SetData(size_t size, const void* data) = 0;
};

} // namespace SmartRenderer
