#pragma once

#include "core/Renderer.h"
#include <memory>
#include <vector>

namespace SmartRenderer {

class Window;
class ANGLEContext;

class OpenGLESRenderer : public Renderer {
public:
    OpenGLESRenderer();
    ~OpenGLESRenderer() override;

    bool Initialize(Window* window, const RendererConfig& config) override;
    void Shutdown() override;

    void BeginFrame() override;
    void EndFrame() override;
    void Present() override;

    void SetViewport(int x, int y, int width, int height) override;
    void Clear(ClearFlags flags, const Color& color = Color::Black,
              float depth = 1.0f, int stencil = 0) override;

    Shader* CreateShader(const std::string& vertexSource,
                        const std::string& fragmentSource) override;
    Texture* CreateTexture(int width, int height, int format,
                          const void* data = nullptr) override;
    Buffer* CreateBuffer(size_t size, const void* data = nullptr) override;
    VertexArray* CreateVertexArray() override;
    Framebuffer* CreateFramebuffer(int width, int height) override;

    void DestroyShader(Shader* shader) override;
    void DestroyTexture(Texture* texture) override;
    void DestroyBuffer(Buffer* buffer) override;
    void DestroyVertexArray(VertexArray* vao) override;
    void DestroyFramebuffer(Framebuffer* framebuffer) override;

    void BindShader(Shader* shader) override;
    void BindTexture(Texture* texture, int unit = 0) override;
    void BindVertexArray(VertexArray* vao) override;
    void BindFramebuffer(Framebuffer* framebuffer) override;

    void DrawArrays(int first, int count) override;
    void DrawElements(int count, int offset = 0) override;

    const RendererCapabilities& GetCapabilities() const override { return m_capabilities; }
    RenderAPI GetAPI() const override { return RenderAPI::OpenGL; }

private:
    Window* m_window;
    std::unique_ptr<ANGLEContext> m_context;
    RendererCapabilities m_capabilities;
    bool m_initialized;

    Shader* m_currentShader;
    Texture* m_currentTextures[32];
    VertexArray* m_currentVAO;
    Framebuffer* m_currentFramebuffer;

    void QueryCapabilities();
};

} // namespace SmartRenderer
