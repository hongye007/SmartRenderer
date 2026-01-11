#pragma once

#include <memory>
#include <string>
#include "math/MathTypes.h"

namespace SmartRenderer {

// Forward declarations
class Window;
class Shader;
class Texture;
class Buffer;
class VertexArray;
class Framebuffer;

// Rendering API types
enum class RenderAPI {
    OpenGL,
    Vulkan,
    Metal,
    Direct3D11,
    Direct3D12
};

// Renderer capabilities
struct RendererCapabilities {
    RenderAPI api = RenderAPI::OpenGL;
    std::string apiVersion;
    std::string vendor;
    std::string renderer;
    int maxTextureUnits = 0;
    int maxTextureSize = 0;
    int maxRenderbufferSize = 0;
    bool supportsInstancing = false;
    bool supportsComputeShaders = false;
    bool supportsGeometryShaders = false;
    bool supportsTessellation = false;
};

// Renderer configuration
struct RendererConfig {
    RenderAPI preferredAPI = RenderAPI::OpenGL;
    bool enableDebug = false;
    bool enableValidation = false;
    int maxFramesInFlight = 2;
    bool vsync = true;
    bool srgb = true;
};

// Clear flags
enum class ClearFlags {
    None = 0,
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
    All = Color | Depth | Stencil
};
inline ClearFlags operator|(ClearFlags a, ClearFlags b) {
    return static_cast<ClearFlags>(static_cast<int>(a) | static_cast<int>(b));
}
inline ClearFlags operator&(ClearFlags a, ClearFlags b) {
    return static_cast<ClearFlags>(static_cast<int>(a) & static_cast<int>(b));
}
inline bool operator!=(ClearFlags a, ClearFlags b) {
    return static_cast<int>(a) != static_cast<int>(b);
}

// Abstract renderer interface
class Renderer {
public:
    virtual ~Renderer() = default;

    virtual bool Initialize(Window* window, const RendererConfig& config) = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;

    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void Clear(ClearFlags flags, const Color& color = Color::Black,
                      float depth = 1.0f, int stencil = 0) = 0;

    virtual Shader* CreateShader(const std::string& vertexSource,
                                const std::string& fragmentSource) = 0;
    virtual Texture* CreateTexture(int width, int height, int format,
                                  const void* data = nullptr) = 0;
    virtual Buffer* CreateBuffer(size_t size, const void* data = nullptr) = 0;
    virtual VertexArray* CreateVertexArray() = 0;
    virtual Framebuffer* CreateFramebuffer(int width, int height) = 0;

    virtual void DestroyShader(Shader* shader) = 0;
    virtual void DestroyTexture(Texture* texture) = 0;
    virtual void DestroyBuffer(Buffer* buffer) = 0;
    virtual void DestroyVertexArray(VertexArray* vao) = 0;
    virtual void DestroyFramebuffer(Framebuffer* framebuffer) = 0;

    virtual void BindShader(Shader* shader) = 0;
    virtual void BindTexture(Texture* texture, int unit = 0) = 0;
    virtual void BindVertexArray(VertexArray* vao) = 0;
    virtual void BindFramebuffer(Framebuffer* framebuffer) = 0;

    virtual void DrawArrays(int first, int count) = 0;
    virtual void DrawElements(int count, int offset = 0) = 0;

    virtual const RendererCapabilities& GetCapabilities() const = 0;
    virtual RenderAPI GetAPI() const = 0;
};

// Factory function
std::unique_ptr<Renderer> CreateRenderer(Window* window, const RendererConfig& config);

} // namespace SmartRenderer
