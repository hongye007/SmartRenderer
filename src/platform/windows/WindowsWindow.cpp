#include "platform/windows/WindowsWindow.h"
// Include GLFW native headers for Windows if needed elsewhere
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>

namespace SmartRenderer {
// WindowsWindow implementation is now in GLFWWindowBase
// This file is kept for potential Windows-specific extensions
} // namespace SmartRenderer
