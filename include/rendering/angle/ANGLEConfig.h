#pragma once

#include "ANGLEContext.h"

namespace SmartRenderer {

// ANGLE configuration utilities
class ANGLEConfigHelper {
public:
    // Get recommended backend for current platform
    static ANGLEConfig::BackendType GetRecommendedBackend();

    // Create default config
    static ANGLEConfig CreateDefault();

    // Create config for specific backend
    static ANGLEConfig CreateForBackend(ANGLEConfig::BackendType backend);
};

} // namespace SmartRenderer
