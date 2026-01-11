#include "SmartRenderer.h"
#include <iostream>

int main() {
    // Initialize platform
    SmartRenderer::PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.appName = "Basic Triangle Sample";

    auto platform = SmartRenderer::CreatePlatform(SmartRenderer::PlatformType::Windows);
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }

    // Create window
    auto window = platform->CreateWindow(800, 600, "Basic Triangle");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    // Initialize renderer
    SmartRenderer::RendererConfig renderConfig;
    auto renderer = SmartRenderer::CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        return 1;
    }

    // Main loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(SmartRenderer::ClearFlags::Color, SmartRenderer::Color(0.2f, 0.3f, 0.4f, 1.0f));
        // Render triangle here
        renderer->EndFrame();
        renderer->Present();
    }

    renderer->Shutdown();
    platform->Shutdown();
    return 0;
}
