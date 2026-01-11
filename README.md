# SmartRenderer

A cross-platform rendering engine supporting Android, iOS, Windows, and macOS.

## Features

- **Cross-platform**: Supports Android, iOS, Windows, and macOS
- **Unified API**: OpenGL ES 3.0+ with ANGLE for automatic platform API conversion
- **Render Graph System**: Configurable rendering pipeline with node-based composition
- **Configurable Protocols**: JSON/Code-driven rendering configuration
- **Modern C++**: C++17 standard

## Architecture

SmartRenderer uses a layered architecture:

```
Application Layer
    ↓
Engine API Layer (Renderer, Platform, Resource)
    ↓
Core Systems (Render Pipeline, Math Library, Asset Manager)
    ↓
Platform Abstraction Layer (Windows, macOS, iOS, Android)
    ↓
Rendering Backend (OpenGL ES → ANGLE → Platform Native API)
```

## Building

### Prerequisites

- CMake 3.16+
- C++17 compatible compiler
- ANGLE library (will be integrated as submodule)
- Android NDK (for Android builds)
- Xcode (for iOS/macOS builds)
- Visual Studio 2019+ (for Windows builds)

### Build Commands

```bash
# Clone repository
git clone <repository-url>
cd SmartRenderer

# Initialize submodules (ANGLE)
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

## Project Structure

```
SmartRenderer/
├── include/          # Public headers
├── src/              # Implementation files
├── third_party/      # External dependencies (ANGLE, etc.)
├── samples/          # Example applications
├── tests/            # Test suite
├── configs/          # Render graph configurations
└── docs/             # Documentation
```

## Quick Start

```cpp
#include "SmartRenderer.h"

int main() {
    // Initialize platform
    SmartRenderer::PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    
    auto platform = SmartRenderer::CreatePlatform(SmartRenderer::PlatformType::Windows);
    platform->Initialize(config);
    
    // Create window
    auto window = platform->CreateWindow(800, 600, "SmartRenderer");
    
    // Initialize renderer
    SmartRenderer::RendererConfig renderConfig;
    auto renderer = SmartRenderer::CreateRenderer(window, renderConfig);
    
    // Main loop
    while (!window->ShouldClose()) {
        platform->PollEvents();
        
        renderer->BeginFrame();
        // Render your scene
        renderer->EndFrame();
        renderer->Present();
    }
    
    return 0;
}
```

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please read the contributing guidelines before submitting PRs.
