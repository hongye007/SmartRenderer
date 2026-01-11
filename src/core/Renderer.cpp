#include "core/Renderer.h"
#include "rendering/opengles/OpenGLESRenderer.h"

namespace SmartRenderer {

std::unique_ptr<Renderer> CreateRenderer(Window* window, const RendererConfig& config) {
    // For now, always create OpenGL ES renderer
    auto renderer = std::make_unique<OpenGLESRenderer>();
    if (renderer->Initialize(window, config)) {
        return renderer;
    }
    return nullptr;
}

} // namespace SmartRenderer
