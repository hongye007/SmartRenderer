#include "SmartRenderer.h"
#include <iostream>

int main() {
    SmartRenderer::PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.appName = "Texture Sample";

    auto platform = SmartRenderer::CreatePlatform(SmartRenderer::PlatformType::Windows);
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }

    auto window = platform->CreateWindow(800, 600, "Texture Sample");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    SmartRenderer::RendererConfig renderConfig;
    auto renderer = SmartRenderer::CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        return 1;
    }

    // Load texture
    SmartRenderer::ResourceManager resourceManager;
    resourceManager.SetRenderer(renderer.get());
    // auto texture = resourceManager.LoadTexture("textures/test.png");

    while (!window->ShouldClose()) {
        platform->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(SmartRenderer::ClearFlags::All, SmartRenderer::Color::Blue);
        // Render texture
        renderer->EndFrame();
        renderer->Present();
    }

    platform->Shutdown();
    return 0;
}
