#pragma once

#include "platform/GLFWWindowBase.h"

namespace SmartRenderer {

// Windows-specific window implementation using GLFW
// All functionality is inherited from GLFWWindowBase
class WindowsWindow : public GLFWWindowBase {
public:
    WindowsWindow() = default;
    ~WindowsWindow() override = default;
};

} // namespace SmartRenderer
