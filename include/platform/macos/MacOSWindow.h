#pragma once

#include "platform/GLFWWindowBase.h"

namespace SmartRenderer {

// macOS-specific window implementation using GLFW
// All functionality is inherited from GLFWWindowBase
class MacOSWindow : public GLFWWindowBase {
public:
    MacOSWindow() = default;
    ~MacOSWindow() override = default;
    
    // Override Show() to add macOS-specific behavior if needed
    void Show() override;
};

} // namespace SmartRenderer
