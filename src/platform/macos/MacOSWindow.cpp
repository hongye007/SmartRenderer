#include "platform/macos/MacOSWindow.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>
#include <stdexcept>

namespace SmartRenderer {

MacOSWindow::MacOSWindow()
    : m_window(nullptr)
    , m_shouldClose(false)
    , m_visible(false) {
    m_handle.nsWindow = nullptr;
    m_handle.nsView = nullptr;
}

MacOSWindow::~MacOSWindow() {
    Destroy();
}

bool MacOSWindow::Create(const WindowConfig& config) {
    if (m_window) {
        return false; // Already created
    }

    m_config = config;

    // Initialize GLFW if not already initialized
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    // Configure GLFW to create EGL context (ANGLE will be used via EGL)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, config.borderless ? GLFW_FALSE : GLFW_TRUE);

    // Create window
    m_window = glfwCreateWindow(config.width, config.height, config.title.c_str(), 
                                 config.fullscreen ? glfwGetPrimaryMonitor() : nullptr, 
                                 nullptr);
    
    if (!m_window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    // GLFW will create EGL context, we can get EGL objects via GLFW API
    // No need to directly access NSWindow/NSView
    // Store GLFW window pointer in handle for ANGLE to use GLFW's EGL functions
    m_handle.nsWindow = m_window;  // Store GLFWwindow* instead of NSWindow*
    m_handle.nsView = nullptr;     // Not needed when using GLFW's EGL API

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);

    // Set callbacks
    glfwSetWindowSizeCallback(m_window, OnWindowResize);
    glfwSetWindowCloseCallback(m_window, OnWindowClose);
    glfwSetWindowFocusCallback(m_window, OnWindowFocus);

    m_shouldClose = false;
    m_visible = false;

    return true;
}

void MacOSWindow::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    m_handle.nsWindow = nullptr;
    m_handle.nsView = nullptr;
    m_visible = false;
}

bool MacOSWindow::IsValid() const {
    return m_window != nullptr;
}

void MacOSWindow::SetTitle(const std::string& title) {
    if (m_window) {
        glfwSetWindowTitle(m_window, title.c_str());
        m_config.title = title;
    }
}

std::string MacOSWindow::GetTitle() const {
    return m_config.title;
}

void MacOSWindow::SetSize(int width, int height) {
    if (m_window) {
        glfwSetWindowSize(m_window, width, height);
        m_config.width = width;
        m_config.height = height;
    }
}

void MacOSWindow::GetSize(int& width, int& height) const {
    if (m_window) {
        glfwGetWindowSize(m_window, &width, &height);
    } else {
        width = m_config.width;
        height = m_config.height;
    }
}

void MacOSWindow::SetPosition(int x, int y) {
    if (m_window) {
        glfwSetWindowPos(m_window, x, y);
    }
}

void MacOSWindow::GetPosition(int& x, int& y) const {
    if (m_window) {
        glfwGetWindowPos(m_window, &x, &y);
    } else {
        x = m_config.x;
        y = m_config.y;
    }
}

void MacOSWindow::Show() {
    if (m_window) {
        glfwShowWindow(m_window);
        m_visible = true;
        
        // Ensure window is fully displayed
        // ANGLE will handle layer setup automatically
        glfwPollEvents();
    }
}

void MacOSWindow::Hide() {
    if (m_window) {
        glfwHideWindow(m_window);
        m_visible = false;
    }
}

bool MacOSWindow::IsVisible() const {
    return m_visible && m_window && glfwGetWindowAttrib(m_window, GLFW_VISIBLE);
}

void MacOSWindow::Minimize() {
    if (m_window) {
        glfwIconifyWindow(m_window);
    }
}

void MacOSWindow::Maximize() {
    if (m_window) {
        glfwMaximizeWindow(m_window);
    }
}

void MacOSWindow::Restore() {
    if (m_window) {
        glfwRestoreWindow(m_window);
    }
}

bool MacOSWindow::IsMinimized() const {
    return m_window && glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
}

bool MacOSWindow::IsMaximized() const {
    // GLFW doesn't have a direct way to check if maximized
    // We can check if window is not iconified and not fullscreen
    return m_window && !glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) && 
           !glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED);
}

void MacOSWindow::SetFullscreen(bool fullscreen) {
    if (!m_window) return;

    if (fullscreen != m_config.fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        if (fullscreen) {
            glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(m_window, nullptr, m_config.x, m_config.y, 
                                m_config.width, m_config.height, 0);
        }
        m_config.fullscreen = fullscreen;
    }
}

bool MacOSWindow::IsFullscreen() const {
    return m_window && glfwGetWindowMonitor(m_window) != nullptr;
}

void MacOSWindow::PollEvents() {
    glfwPollEvents();
}

bool MacOSWindow::ShouldClose() const {
    return m_shouldClose || (m_window && glfwWindowShouldClose(m_window));
}

void MacOSWindow::SetResizeCallback(WindowResizeCallback callback) {
    m_resizeCallback = callback;
}

void MacOSWindow::SetCloseCallback(WindowCloseCallback callback) {
    m_closeCallback = callback;
}

void MacOSWindow::SetFocusCallback(WindowFocusCallback callback) {
    m_focusCallback = callback;
}

WindowHandle* MacOSWindow::GetNativeHandle() const {
    return const_cast<WindowHandle*>(&m_handle);
}

void MacOSWindow::GetFramebufferSize(int& width, int& height) const {
    if (m_window) {
        glfwGetFramebufferSize(m_window, &width, &height);
    } else {
        width = m_config.width;
        height = m_config.height;
    }
}

float MacOSWindow::GetContentScaleX() const {
    if (m_window) {
        float xscale, yscale;
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        return xscale;
    }
    return 1.0f;
}

float MacOSWindow::GetContentScaleY() const {
    if (m_window) {
        float xscale, yscale;
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        return yscale;
    }
    return 1.0f;
}

void MacOSWindow::SwapBuffers() {
    // GLFW handles buffer swapping when using OpenGL context
    // Since we're using ANGLE/EGL, we don't swap here
    // The renderer will call eglSwapBuffers
}

// Static callbacks
void MacOSWindow::OnWindowResize(GLFWwindow* window, int width, int height) {
    MacOSWindow* self = static_cast<MacOSWindow*>(glfwGetWindowUserPointer(window));
    if (self && self->m_resizeCallback) {
        self->m_resizeCallback(width, height);
    }
    if (self) {
        self->m_config.width = width;
        self->m_config.height = height;
    }
}

void MacOSWindow::OnWindowClose(GLFWwindow* window) {
    MacOSWindow* self = static_cast<MacOSWindow*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_shouldClose = true;
        if (self->m_closeCallback) {
            self->m_closeCallback();
        }
    }
}

void MacOSWindow::OnWindowFocus(GLFWwindow* window, int focused) {
    MacOSWindow* self = static_cast<MacOSWindow*>(glfwGetWindowUserPointer(window));
    if (self && self->m_focusCallback) {
        self->m_focusCallback(focused == GLFW_TRUE);
    }
}

} // namespace SmartRenderer
