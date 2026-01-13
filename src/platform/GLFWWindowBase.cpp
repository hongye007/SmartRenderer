#include "platform/GLFWWindowBase.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace SmartRenderer {

GLFWWindowBase::GLFWWindowBase()
    : m_window(nullptr)
    , m_shouldClose(false)
    , m_visible(false) {
    m_handle.window = nullptr;
}

GLFWWindowBase::~GLFWWindowBase() {
    Destroy();
}

bool GLFWWindowBase::Create(const WindowConfig& config) {
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
    // Store GLFW window pointer in handle for ANGLE to use GLFW's EGL functions
    m_handle.window = m_window;  // Store GLFWwindow*

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

void GLFWWindowBase::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    m_handle.window = nullptr;
    m_visible = false;
}

bool GLFWWindowBase::IsValid() const {
    return m_window != nullptr;
}

void GLFWWindowBase::SetTitle(const std::string& title) {
    if (m_window) {
        glfwSetWindowTitle(m_window, title.c_str());
        m_config.title = title;
    }
}

std::string GLFWWindowBase::GetTitle() const {
    return m_config.title;
}

void GLFWWindowBase::SetSize(int width, int height) {
    if (m_window) {
        glfwSetWindowSize(m_window, width, height);
        m_config.width = width;
        m_config.height = height;
    }
}

void GLFWWindowBase::GetSize(int& width, int& height) const {
    if (m_window) {
        glfwGetWindowSize(m_window, &width, &height);
    } else {
        width = m_config.width;
        height = m_config.height;
    }
}

void GLFWWindowBase::SetPosition(int x, int y) {
    if (m_window) {
        glfwSetWindowPos(m_window, x, y);
        m_config.x = x;
        m_config.y = y;
    }
}

void GLFWWindowBase::GetPosition(int& x, int& y) const {
    if (m_window) {
        glfwGetWindowPos(m_window, &x, &y);
    } else {
        x = m_config.x;
        y = m_config.y;
    }
}

void GLFWWindowBase::Show() {
    if (m_window) {
        glfwShowWindow(m_window);
        m_visible = true;
    }
}

void GLFWWindowBase::Hide() {
    if (m_window) {
        glfwHideWindow(m_window);
        m_visible = false;
    }
}

bool GLFWWindowBase::IsVisible() const {
    return m_visible && m_window && glfwGetWindowAttrib(m_window, GLFW_VISIBLE);
}

void GLFWWindowBase::Minimize() {
    if (m_window) {
        glfwIconifyWindow(m_window);
    }
}

void GLFWWindowBase::Maximize() {
    if (m_window) {
        glfwMaximizeWindow(m_window);
    }
}

void GLFWWindowBase::Restore() {
    if (m_window) {
        glfwRestoreWindow(m_window);
    }
}

bool GLFWWindowBase::IsMinimized() const {
    return m_window && glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
}

bool GLFWWindowBase::IsMaximized() const {
    // GLFW doesn't have a direct way to check if maximized
    return m_window && !glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) && 
           glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED);
}

void GLFWWindowBase::SetFullscreen(bool fullscreen) {
    if (!m_window) {
        return;
    }

    m_config.fullscreen = fullscreen;
    
    if (fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
    } else {
        glfwSetWindowMonitor(m_window, nullptr, m_config.x, m_config.y, 
                            m_config.width, m_config.height, GLFW_DONT_CARE);
    }
}

bool GLFWWindowBase::IsFullscreen() const {
    return m_window && glfwGetWindowMonitor(m_window) != nullptr;
}

void GLFWWindowBase::PollEvents() {
    glfwPollEvents();
}

bool GLFWWindowBase::ShouldClose() const {
    return m_shouldClose || (m_window && glfwWindowShouldClose(m_window));
}

void GLFWWindowBase::SetResizeCallback(WindowResizeCallback callback) {
    m_resizeCallback = callback;
}

void GLFWWindowBase::SetCloseCallback(WindowCloseCallback callback) {
    m_closeCallback = callback;
}

void GLFWWindowBase::SetFocusCallback(WindowFocusCallback callback) {
    m_focusCallback = callback;
}

WindowHandle* GLFWWindowBase::GetNativeHandle() const {
    return const_cast<WindowHandle*>(&m_handle);
}

void GLFWWindowBase::GetFramebufferSize(int& width, int& height) const {
    if (m_window) {
        glfwGetFramebufferSize(m_window, &width, &height);
    } else {
        width = m_config.width;
        height = m_config.height;
    }
}

float GLFWWindowBase::GetContentScaleX() const {
    if (m_window) {
        float xscale, yscale;
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        return xscale;
    }
    return 1.0f;
}

float GLFWWindowBase::GetContentScaleY() const {
    if (m_window) {
        float xscale, yscale;
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        return yscale;
    }
    return 1.0f;
}

void GLFWWindowBase::SwapBuffers() {
    // GLFW handles buffer swapping when using OpenGL context
    // Since we're using ANGLE/EGL, we don't swap here
    // The renderer will call eglSwapBuffers
}

GLFWWindowBase* GLFWWindowBase::GetWindowInstance(GLFWwindow* window) {
    return static_cast<GLFWWindowBase*>(glfwGetWindowUserPointer(window));
}

void GLFWWindowBase::OnWindowResize(GLFWwindow* window, int width, int height) {
    GLFWWindowBase* self = GetWindowInstance(window);
    if (self) {
        self->m_config.width = width;
        self->m_config.height = height;
        if (self->m_resizeCallback) {
            self->m_resizeCallback(width, height);
        }
    }
}

void GLFWWindowBase::OnWindowClose(GLFWwindow* window) {
    GLFWWindowBase* self = GetWindowInstance(window);
    if (self) {
        self->m_shouldClose = true;
        if (self->m_closeCallback) {
            self->m_closeCallback();
        }
    }
}

void GLFWWindowBase::OnWindowFocus(GLFWwindow* window, int focused) {
    GLFWWindowBase* self = GetWindowInstance(window);
    if (self && self->m_focusCallback) {
        self->m_focusCallback(focused == GLFW_TRUE);
    }
}

} // namespace SmartRenderer
