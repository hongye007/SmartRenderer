#pragma once

#include "core/Renderer.h"
#include <memory>
#include <functional>

namespace SmartRenderer {

// Forward declarations
class Shader;
class Texture;
class Buffer;
class VertexArray;
class Framebuffer;

// Resource handle with automatic lifetime management
// Automatically destroys the resource when the handle is destroyed
// Uses shared_ptr for reference counting and automatic cleanup
template<typename ResourceType>
class ResourceHandle {
public:
    ResourceHandle() : m_resource(nullptr) {}
    
    ResourceHandle(ResourceType* resource, Renderer* renderer, 
                   std::function<void(Renderer*, ResourceType*)> destroyFunc) {
        if (resource && renderer && destroyFunc) {
            // Use shared_ptr with custom deleter
            m_resource = std::shared_ptr<ResourceType>(
                resource,
                [renderer, destroyFunc](ResourceType* res) {
                    if (renderer && res && destroyFunc) {
                        destroyFunc(renderer, res);
                    }
                }
            );
        } else {
            m_resource = nullptr;
        }
    }
    
    // Copy constructor (default, uses shared_ptr's copy semantics)
    ResourceHandle(const ResourceHandle&) = default;
    
    // Move constructor
    ResourceHandle(ResourceHandle&&) noexcept = default;
    
    // Copy assignment
    ResourceHandle& operator=(const ResourceHandle&) = default;
    
    // Move assignment
    ResourceHandle& operator=(ResourceHandle&&) noexcept = default;
    
    // Access the underlying resource
    ResourceType* Get() const { return m_resource.get(); }
    ResourceType* operator->() const { return m_resource.get(); }
    ResourceType& operator*() const { return *m_resource; }
    
    // Check if handle is valid
    bool IsValid() const { return m_resource != nullptr; }
    explicit operator bool() const { return IsValid(); }
    
    // Get shared_ptr (for advanced usage)
    std::shared_ptr<ResourceType> GetShared() const { return m_resource; }
    
    // Reset the handle
    void Reset() {
        m_resource.reset();
    }

private:
    std::shared_ptr<ResourceType> m_resource;
};

// Type aliases for convenience
using ShaderHandle = ResourceHandle<Shader>;
using TextureHandle = ResourceHandle<Texture>;
using BufferHandle = ResourceHandle<Buffer>;
using VertexArrayHandle = ResourceHandle<VertexArray>;
using FramebufferHandle = ResourceHandle<Framebuffer>;

} // namespace SmartRenderer
