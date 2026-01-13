#include "SmartRenderer.h"
#include "platform/Platform.h"
#include "core/Renderer.h"

namespace SmartRenderer {

static Platform* g_platform = nullptr;
static Renderer* g_renderer = nullptr;

bool Initialize(const PlatformConfig& config) {
    // Use auto-detection if type is not specified or matches current platform
    g_platform = CreatePlatform().release();
    if (!g_platform || !g_platform->Initialize(config)) {
        return false;
    }
    return true;
}

void Shutdown() {
    if (g_renderer) {
        g_renderer->Shutdown();
        delete g_renderer;
        g_renderer = nullptr;
    }
    if (g_platform) {
        g_platform->Shutdown();
        delete g_platform;
        g_platform = nullptr;
    }
}

Platform* GetPlatform() {
    return g_platform;
}

Renderer* GetRenderer() {
    return g_renderer;
}

} // namespace SmartRenderer
