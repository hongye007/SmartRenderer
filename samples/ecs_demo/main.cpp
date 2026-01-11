#include "SmartRenderer.h"
#include "ecs/ECSProtocol.h"
#include "core/RenderContext.h"
#include <iostream>
#include <memory>

using namespace SmartRenderer;

int main() {
    // Initialize platform
    PlatformConfig config;
    config.windowWidth = 1024;
    config.windowHeight = 768;
    config.appName = "ECS Demo";
    
    auto platform = CreatePlatform(PlatformType::Windows);
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
        
        // Update ECS World (this will render entities)
        world->Update(deltaTime);
        
        // End frame and present
        renderer->EndFrame();
        renderer->Present();
    }
    
    // Cleanup
    renderer->Shutdown();
    platform->Shutdown();
    
    std::cout << "ECS Demo shutting down..." << std::endl;
    
    return 0;
}
