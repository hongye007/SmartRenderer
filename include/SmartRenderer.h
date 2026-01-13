#pragma once

// Main header for SmartRenderer
#include "core/Renderer.h"
#include "platform/Platform.h"
#include "platform/Window.h"
#include "resource/ResourceManager.h"
#include "math/MathTypes.h"

namespace SmartRenderer {

    // Engine version
    static constexpr int VERSION_MAJOR = 1;
    static constexpr int VERSION_MINOR = 0;
    static constexpr int VERSION_PATCH = 0;

    // Initialize the rendering engine
    bool Initialize(const PlatformConfig& config);

    // Shutdown the rendering engine
    void Shutdown();

    // Get the current platform
    Platform* GetPlatform();

    // Get the current renderer
    Renderer* GetRenderer();

}
