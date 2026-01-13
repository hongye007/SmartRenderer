#include "SmartRenderer.h"
#include "ecs/ECSProtocol.h"
#include "ecs/systems/RenderSystem.h"
#include "core/RenderContext.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace SmartRenderer;

int main() {
    // Initialize platform
    PlatformConfig config;
    config.windowWidth = 1024;
    config.windowHeight = 768;
    config.appName = "ECS Demo";
    
    auto platform = CreatePlatform(); // Auto-detect platform
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }
    
    // Create window
    auto window = platform->CreateWindow(1024, 768, "ECS Demo - SmartRenderer");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }
    
    window->Show();
    
    // Create renderer
    RendererConfig renderConfig;
    auto renderer = CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        return 1;
    }
    
    // Create render context
    RenderContext context(renderer.get());
    
    // Load ECS configuration from JSON
    ECSProtocol ecsProtocol(renderer.get());
    std::string configPath = "configs/ecs_demo.json";
    
    if (!ecsProtocol.LoadFromFile(configPath)) {
        std::cerr << "Failed to load ECS configuration from: " << configPath << std::endl;
        std::cerr << "Make sure the config file exists relative to the executable" << std::endl;
        return 1;
    }
    
    World* world = ecsProtocol.GetWorld();
    if (!world) {
        std::cerr << "Failed to get ECS World from protocol" << std::endl;
        return 1;
    }
    
    std::cout << "ECS Demo initialized from JSON protocol:" << std::endl;
    std::cout << "  - Configuration file: " << configPath << std::endl;
    std::cout << "  - Using JSON-based ECS configuration" << std::endl;
    std::cout << std::endl;
    std::cout << "Rendering two triangles using ECS:" << std::endl;
    std::cout << "  - Left triangle: Blue" << std::endl;
    std::cout << "  - Right triangle: Red" << std::endl;
    std::cout << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;
    
    // Main loop
    float deltaTime = 0.016f; // ~60 FPS
    
    while (!window->ShouldClose()) {
        // Process window events
        window->PollEvents();
        
        // Begin frame
        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        // Update ECS World (this will generate render commands)
        world->Update(deltaTime);
        
        // Execute render commands from RenderSystem
        // Find RenderSystem and execute its command queue
        // Note: In a full implementation, we'd have a better way to get the RenderSystem
        // For now, we'll need to get it from the world or store a reference
        // This is a temporary solution - in production, you'd want a better system registry
        auto* renderSystem = world->GetSystem<RenderSystem>();
        if (renderSystem) {
            renderSystem->GetCommandQueue().Execute(*renderer);
        }
        
        // End frame and present
        renderer->EndFrame();
        renderer->Present();
    }
    
    // Cleanup order is critical:
    // 1. Unbind all resources first
    renderer->BindShader(nullptr);
    renderer->BindTexture(nullptr, 0);
    renderer->BindVertexArray(nullptr);
    
    // 2. Clear World before shutting down renderer
    //    This ensures Material components (with ShaderHandle) are destroyed
    //    while renderer is still valid
    if (world) {
        world->Clear();
    }
    
    // 3. Shutdown renderer (after all resources are destroyed)
    //    Note: ecsProtocol will be destroyed when going out of scope,
    //    but World is already cleared, so Material components won't try to
    //    destroy resources after renderer is shut down
    renderer->Shutdown();
    renderer.reset();
    
    // 4. Destroy window
    window->Destroy();
    
    // 5. Shutdown platform
    platform->Shutdown();
    platform.reset();
    
    // 6. ecsProtocol will be destroyed here when going out of scope
    //    But World is already cleared, so it's safe
    
    std::cout << "ECS Demo shutting down..." << std::endl;
    
    return 0;
}
