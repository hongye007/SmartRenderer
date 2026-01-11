#pragma once

#include "core/Renderer.h"

namespace SmartRenderer {

class RenderContext {
public:
    RenderContext(Renderer* renderer) : m_renderer(renderer) {}
    Renderer* GetRenderer() { return m_renderer; }

private:
    Renderer* m_renderer;
};

} // namespace SmartRenderer
