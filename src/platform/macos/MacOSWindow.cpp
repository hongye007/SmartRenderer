#include "platform/macos/MacOSWindow.h"
#include <GLFW/glfw3.h>
// Include GLFW native headers for macOS if needed elsewhere
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>

namespace SmartRenderer {

void MacOSWindow::Show() {
    // Call base class implementation
    GLFWWindowBase::Show();
    
    // macOS-specific: Ensure window is fully displayed
    // ANGLE will handle layer setup automatically
    if (m_window) {
        glfwPollEvents();
    }
}

} // namespace SmartRenderer
