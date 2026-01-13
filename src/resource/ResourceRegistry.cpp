#include "resource/ResourceRegistry.h"
#include "core/Renderer.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/Buffer.h"
#include "rendering/VertexArray.h"
#include "rendering/Framebuffer.h"

namespace SmartRenderer {

ResourceRegistry::ResourceRegistry(Renderer* renderer)
    : m_renderer(renderer) {
}

ResourceRegistry::~ResourceRegistry() {
    Clear();
}

// Register shader
ShaderHandle ResourceRegistry::RegisterShader(const std::string& name, Shader* shader) {
    if (!shader || !m_renderer) {
        return ShaderHandle();
    }

    // Capture renderer pointer - deleter will check if it's still valid
    Renderer* rendererPtr = m_renderer;
    auto deleter = [rendererPtr](Renderer* renderer, Shader* s) {
        // Only destroy if renderer is still valid (not nullptr)
        // This prevents crashes when renderer is destroyed before resources
        if (renderer && renderer == rendererPtr && s) {
            try {
                renderer->DestroyShader(s);
            } catch (...) {
                // Ignore exceptions during cleanup
            }
        }
    };

    ShaderHandle handle(shader, m_renderer, deleter);
    m_shaders[name] = handle;
    return handle;
}

// Register texture
TextureHandle ResourceRegistry::RegisterTexture(const std::string& name, Texture* texture) {
    if (!texture || !m_renderer) {
        return TextureHandle();
    }

    Renderer* rendererPtr = m_renderer;
    auto deleter = [rendererPtr](Renderer* renderer, Texture* t) {
        if (renderer && renderer == rendererPtr && t) {
            try {
                renderer->DestroyTexture(t);
            } catch (...) {
                // Ignore exceptions during cleanup
            }
        }
    };

    TextureHandle handle(texture, m_renderer, deleter);
    m_textures[name] = handle;
    return handle;
}

// Register buffer
BufferHandle ResourceRegistry::RegisterBuffer(const std::string& name, Buffer* buffer) {
    if (!buffer || !m_renderer) {
        return BufferHandle();
    }

    Renderer* rendererPtr = m_renderer;
    auto deleter = [rendererPtr](Renderer* renderer, Buffer* b) {
        if (renderer && renderer == rendererPtr && b) {
            try {
                renderer->DestroyBuffer(b);
            } catch (...) {
                // Ignore exceptions during cleanup
            }
        }
    };

    BufferHandle handle(buffer, m_renderer, deleter);
    m_buffers[name] = handle;
    return handle;
}

// Register vertex array
VertexArrayHandle ResourceRegistry::RegisterVertexArray(const std::string& name, VertexArray* vao) {
    if (!vao || !m_renderer) {
        return VertexArrayHandle();
    }

    Renderer* rendererPtr = m_renderer;
    auto deleter = [rendererPtr](Renderer* renderer, VertexArray* v) {
        if (renderer && renderer == rendererPtr && v) {
            try {
                renderer->DestroyVertexArray(v);
            } catch (...) {
                // Ignore exceptions during cleanup
            }
        }
    };

    VertexArrayHandle handle(vao, m_renderer, deleter);
    m_vertexArrays[name] = handle;
    return handle;
}

// Register framebuffer
FramebufferHandle ResourceRegistry::RegisterFramebuffer(const std::string& name, Framebuffer* framebuffer) {
    if (!framebuffer || !m_renderer) {
        return FramebufferHandle();
    }

    Renderer* rendererPtr = m_renderer;
    auto deleter = [rendererPtr](Renderer* renderer, Framebuffer* f) {
        if (renderer && renderer == rendererPtr && f) {
            try {
                renderer->DestroyFramebuffer(f);
            } catch (...) {
                // Ignore exceptions during cleanup
            }
        }
    };

    FramebufferHandle handle(framebuffer, m_renderer, deleter);
    m_framebuffers[name] = handle;
    return handle;
}

// Get shader
ShaderHandle ResourceRegistry::GetShader(const std::string& name) const {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    return ShaderHandle();
}

// Get texture
TextureHandle ResourceRegistry::GetTexture(const std::string& name) const {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    return TextureHandle();
}

// Get buffer
BufferHandle ResourceRegistry::GetBuffer(const std::string& name) const {
    auto it = m_buffers.find(name);
    if (it != m_buffers.end()) {
        return it->second;
    }
    return BufferHandle();
}

// Get vertex array
VertexArrayHandle ResourceRegistry::GetVertexArray(const std::string& name) const {
    auto it = m_vertexArrays.find(name);
    if (it != m_vertexArrays.end()) {
        return it->second;
    }
    return VertexArrayHandle();
}

// Get framebuffer
FramebufferHandle ResourceRegistry::GetFramebuffer(const std::string& name) const {
    auto it = m_framebuffers.find(name);
    if (it != m_framebuffers.end()) {
        return it->second;
    }
    return FramebufferHandle();
}

// Check if resource exists
bool ResourceRegistry::HasShader(const std::string& name) const {
    return m_shaders.find(name) != m_shaders.end();
}

bool ResourceRegistry::HasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

bool ResourceRegistry::HasBuffer(const std::string& name) const {
    return m_buffers.find(name) != m_buffers.end();
}

bool ResourceRegistry::HasVertexArray(const std::string& name) const {
    return m_vertexArrays.find(name) != m_vertexArrays.end();
}

bool ResourceRegistry::HasFramebuffer(const std::string& name) const {
    return m_framebuffers.find(name) != m_framebuffers.end();
}

// Unregister resources
void ResourceRegistry::UnregisterShader(const std::string& name) {
    m_shaders.erase(name);
}

void ResourceRegistry::UnregisterTexture(const std::string& name) {
    m_textures.erase(name);
}

void ResourceRegistry::UnregisterBuffer(const std::string& name) {
    m_buffers.erase(name);
}

void ResourceRegistry::UnregisterVertexArray(const std::string& name) {
    m_vertexArrays.erase(name);
}

void ResourceRegistry::UnregisterFramebuffer(const std::string& name) {
    m_framebuffers.erase(name);
}

// Clear all resources
void ResourceRegistry::Clear() {
    // Clear all maps - handles will automatically destroy resources
    // Resources will be destroyed via their deleters while renderer is still valid
    m_shaders.clear();
    m_textures.clear();
    m_buffers.clear();
    m_vertexArrays.clear();
    m_framebuffers.clear();
}

// Invalidate renderer
void ResourceRegistry::InvalidateRenderer() {
    // Set renderer to nullptr to prevent resource handles from trying to destroy resources
    // after renderer is destroyed. This should be called before renderer->Shutdown()
    m_renderer = nullptr;
}

} // namespace SmartRenderer
