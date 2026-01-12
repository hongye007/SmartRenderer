#include "rendering/opengles/OpenGLESRenderer.h"
#include "rendering/opengles/OpenGLESResourceFactory.h"
#include "rendering/opengles/OpenGLESTypes.h"
#include "rendering/angle/ANGLEContext.h"
#include "platform/Window.h"
#include <stdexcept>

#ifdef USE_ANGLE
#include <GLES3/gl3.h>
#elif defined(__APPLE__)
#include <OpenGL/gl3.h>
#endif

namespace SmartRenderer {

OpenGLESRenderer::OpenGLESRenderer()
    : m_window(nullptr)
    , m_initialized(false)
    , m_currentShader(nullptr)
    , m_currentVAO(nullptr)
    , m_currentFramebuffer(nullptr) {
    for (int i = 0; i < 32; ++i) {
        m_currentTextures[i] = nullptr;
    }
}

OpenGLESRenderer::~OpenGLESRenderer() {
    Shutdown();
}

bool OpenGLESRenderer::Initialize(Window* window, const RendererConfig& config) {
    if (m_initialized) {
        return true;
    }

    m_window = window;
    if (!window) {
        return false;
    }

    // Initialize ANGLE context
    m_context = std::make_unique<ANGLEContext>();
    ANGLEConfig angleConfig;
    angleConfig.majorVersion = 3;
    angleConfig.minorVersion = 0;
    angleConfig.debug = config.enableDebug;

    if (!m_context->Initialize(window, angleConfig)) {
        // Log error
        fprintf(stderr, "Failed to initialize ANGLE context\n");
        return false;
    }

    // Query capabilities
    QueryCapabilities();

    m_initialized = true;
    return true;
}

void OpenGLESRenderer::Shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_context) {
        m_context->Shutdown();
        m_context.reset();
    }

    m_initialized = false;
}

void OpenGLESRenderer::BeginFrame() {
    // Ensure context is current (important for GLFW EGL)
    if (m_context) {
        m_context->MakeCurrent();
    }
    
    // Set viewport to match window size
    if (m_window) {
        int width, height;
        m_window->GetFramebufferSize(width, height);
        if (width > 0 && height > 0) {
            glViewport(0, 0, width, height);
        }
    }
    
    // Clear state
    m_currentShader = nullptr;
    m_currentVAO = nullptr;
    m_currentFramebuffer = nullptr;
}

void OpenGLESRenderer::EndFrame() {
    // End of frame operations
}

void OpenGLESRenderer::Present() {
    if (m_context) {
        m_context->SwapBuffers();
    }
}

void OpenGLESRenderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLESRenderer::Clear(ClearFlags flags, const Color& color, float depth, int stencil) {
    GLbitfield clearFlags = 0;
    if ((flags & ClearFlags::Color) != ClearFlags::None) {
        glClearColor(color.r, color.g, color.b, color.a);
        clearFlags |= GL_COLOR_BUFFER_BIT;
    }
    if ((flags & ClearFlags::Depth) != ClearFlags::None) {
        glClearDepthf(depth);
        clearFlags |= GL_DEPTH_BUFFER_BIT;
    }
    if ((flags & ClearFlags::Stencil) != ClearFlags::None) {
        glClearStencil(stencil);
        clearFlags |= GL_STENCIL_BUFFER_BIT;
    }
    if (clearFlags != 0) {
        glClear(clearFlags);
    }
}

Shader* OpenGLESRenderer::CreateShader(const std::string& vertexSource,
                                      const std::string& fragmentSource) {
    return CreateOpenGLESShader(vertexSource, fragmentSource);
}

Texture* OpenGLESRenderer::CreateTexture(int width, int height, int format,
                                        const void* data) {
    return CreateOpenGLESTexture(width, height, format, data);
}

Buffer* OpenGLESRenderer::CreateBuffer(size_t size, const void* data) {
    return CreateOpenGLESBuffer(size, data, BufferUsage::Static);
}

VertexArray* OpenGLESRenderer::CreateVertexArray() {
    return CreateOpenGLESVertexArray();
}

Framebuffer* OpenGLESRenderer::CreateFramebuffer(int width, int height) {
    return CreateOpenGLESFramebuffer(width, height);
}

void OpenGLESRenderer::DestroyShader(Shader* shader) {
    if (shader) {
        delete shader;
    }
}

void OpenGLESRenderer::DestroyTexture(Texture* texture) {
    if (texture) {
        delete texture;
    }
}

void OpenGLESRenderer::DestroyBuffer(Buffer* buffer) {
    if (buffer) {
        delete buffer;
    }
}

void OpenGLESRenderer::DestroyVertexArray(VertexArray* vao) {
    if (vao) {
        delete vao;
    }
}

void OpenGLESRenderer::DestroyFramebuffer(Framebuffer* framebuffer) {
    if (framebuffer) {
        delete framebuffer;
    }
}

void OpenGLESRenderer::BindShader(Shader* shader) {
    m_currentShader = shader;
    if (shader) {
        shader->Bind();
    } else {
        glUseProgram(0);
    }
}

void OpenGLESRenderer::BindTexture(Texture* texture, int unit) {
    if (unit >= 0 && unit < 32) {
        m_currentTextures[unit] = texture;
        if (texture) {
            texture->Bind(unit);
        } else {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void OpenGLESRenderer::BindVertexArray(VertexArray* vao) {
    m_currentVAO = vao;
    if (vao) {
        vao->Bind();
    } else {
        glBindVertexArray(0);
    }
}

void OpenGLESRenderer::BindFramebuffer(Framebuffer* framebuffer) {
    m_currentFramebuffer = framebuffer;
    if (framebuffer) {
        framebuffer->Bind();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void OpenGLESRenderer::DrawArrays(int first, int count) {
    glDrawArrays(GL_TRIANGLES, first, count);
}

void OpenGLESRenderer::DrawElements(int count, int offset) {
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<const void*>(offset * sizeof(unsigned int)));
}

void OpenGLESRenderer::QueryCapabilities() {
    m_capabilities.api = RenderAPI::OpenGL;
    m_capabilities.apiVersion = "OpenGL ES 3.0";
    m_capabilities.vendor = "ANGLE";
    m_capabilities.renderer = "ANGLE Renderer";
    m_capabilities.maxTextureUnits = 32;
    m_capabilities.maxTextureSize = 4096;
    m_capabilities.maxRenderbufferSize = 4096;
    m_capabilities.supportsInstancing = true;
    m_capabilities.supportsComputeShaders = false;
    m_capabilities.supportsGeometryShaders = false;
    m_capabilities.supportsTessellation = false;
}

} // namespace SmartRenderer
