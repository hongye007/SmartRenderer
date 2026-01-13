#include "SmartRenderer.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== Window Test Sample ===" << std::endl;
    std::cout << "SmartRenderer v" << SmartRenderer::VERSION_MAJOR << "." 
              << SmartRenderer::VERSION_MINOR << "." << SmartRenderer::VERSION_PATCH << std::endl;
    
    // Initialize platform
    SmartRenderer::PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.appName = "Window Test";

    auto platform = SmartRenderer::CreatePlatform(); // Auto-detect platform
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }

    std::cout << "Platform initialized: " << platform->GetPlatformName() << std::endl;
    std::cout << "Platform type: " << (platform->IsMobile() ? "Mobile" : "Desktop") << std::endl;

    // Create window
    auto window = platform->CreateWindow(800, 600, "SmartRenderer - Window Test");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    window->Show();
    std::cout << "Window created successfully" << std::endl;
    std::cout << "Window title: " << window->GetTitle() << std::endl;
    
    int width, height;
    window->GetSize(width, height);
    std::cout << "Window size: " << width << "x" << height << std::endl;

    // Get platform information
    std::cout << "Screen width: " << platform->GetScreenWidth() << std::endl;
    std::cout << "Screen height: " << platform->GetScreenHeight() << std::endl;
    std::cout << "Screen density: " << platform->GetScreenDensity() << std::endl;

    // Test file system
    auto fileSystem = platform->GetFileSystem();
    std::cout << "Writable path: " << platform->GetWritablePath("") << std::endl;
    std::cout << "Resource path: " << platform->GetResourcePath("") << std::endl;

    // Main loop
    std::cout << "\nWindow is now open. Press ESC or close the window to exit." << std::endl;
    std::cout << "Loop is running..." << std::endl;
    
    int frameCount = 0;
    double startTime = platform->GetCurrentTime();

    while (!window->ShouldClose()) {
        window->PollEvents();
        
        // Simple frame timing
        frameCount++;
        double currentTime = platform->GetCurrentTime();
        double elapsed = currentTime - startTime;
        
        if (elapsed >= 1.0) {
            std::cout << "FPS: " << frameCount << " (Window is responsive)" << std::endl;
            frameCount = 0;
            startTime = currentTime;
        }

        // Small delay to prevent busy loop
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    std::cout << "\nShutting down..." << std::endl;
    platform->DestroyWindow(window);
    platform->Shutdown();
    
    std::cout << "Window test completed successfully!" << std::endl;
    return 0;
}
