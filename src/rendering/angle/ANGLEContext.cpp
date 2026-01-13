#include "rendering/angle/ANGLEContext.h"
#include "platform/Window.h"
#include <stdexcept>
#include <cstring>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
// GLFW only for Apple and Windows platforms
#if defined(__APPLE__) || defined(_WIN32)
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_EGL
#elif defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_EGL
#endif
#include <GLFW/glfw3native.h>
#endif

namespace SmartRenderer {

ANGLEContext::ANGLEContext()
    : m_display(EGL_NO_DISPLAY)
    , m_context(EGL_NO_CONTEXT)
    , m_surface(EGL_NO_SURFACE)
    , m_config(nullptr)
    , m_initialized(false)
    , m_usingGLFW(false) {
}

ANGLEContext::~ANGLEContext() {
    Shutdown();
}

bool ANGLEContext::Initialize(Window* window, const ANGLEConfig& config) {
    if (m_initialized) {
        return true;
    }

    if (!window) {
        return false;
    }

    ConfigureBackend(config);

    #if defined(__APPLE__) || defined(_WIN32)
    // On macOS and Windows, we require GLFW
    WindowHandle* handle = window->GetNativeHandle();
    if (!handle || !handle->window) {
        fprintf(stderr, "Failed to get window handle (required on macOS/Windows)\n");
        return false;
    }

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(handle->window);
    if (!glfwWindow) {
        fprintf(stderr, "Failed to get GLFW window (required on macOS/Windows)\n");
        return false;
    }
    
    // Get GLFW's EGL context and surface
    EGLDisplay glfwDisplay = glfwGetEGLDisplay();
    EGLContext glfwContext = glfwGetEGLContext(glfwWindow);
    EGLSurface glfwSurface = glfwGetEGLSurface(glfwWindow);
    
    // On macOS/Windows, GLFW must provide EGL context and surface
    if (glfwDisplay == EGL_NO_DISPLAY || glfwContext == EGL_NO_CONTEXT || glfwSurface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to get GLFW's EGL context/surface (required on macOS/Windows)\n");
        return false;
    }
    
    m_display = glfwDisplay;
    m_context = glfwContext;
    m_surface = glfwSurface;
    m_usingGLFW = true;
    
    m_initialized = true;
    fprintf(stderr, "Using GLFW's EGL context\n");
    return MakeCurrent();
    #else
    // On Android/iOS, manually initialize EGL
    if (!InitializeEGL() || !CreateContext() || !CreateSurface(window)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return false;
    }

    m_initialized = true;
    fprintf(stderr, "Using our own EGL context\n");
    return MakeCurrent();
    #endif
}

void ANGLEContext::Shutdown() {
    if (!m_initialized) {
        return;
    }

    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    
    // Only destroy EGL objects if we created them (not using GLFW's)
    if (!m_usingGLFW) {
    if (m_context != EGL_NO_CONTEXT) {
        eglDestroyContext(m_display, m_context);
    }
    if (m_surface != EGL_NO_SURFACE) {
        eglDestroySurface(m_display, m_surface);
    }
    if (m_display != EGL_NO_DISPLAY) {
        eglTerminate(m_display);
    }
    }

    m_display = EGL_NO_DISPLAY;
    m_context = EGL_NO_CONTEXT;
    m_surface = EGL_NO_SURFACE;
    m_config = nullptr;
    m_initialized = false;
    m_usingGLFW = false;
}

bool ANGLEContext::MakeCurrent() {
    if (!m_initialized) {
        return false;
    }
    
    return eglMakeCurrent(m_display, m_surface, m_surface, m_context) == EGL_TRUE;
}

void ANGLEContext::SwapBuffers() {
    if (!m_initialized) {
        return;
    }

    eglSwapBuffers(m_display, m_surface);
}

void ANGLEContext::SetSwapInterval(int interval) {
    if (!m_initialized) {
        return;
    }

    eglSwapInterval(m_display, interval);
}

ANGLEConfig::BackendType ANGLEContext::GetDefaultBackend() {
#ifdef _WIN32
    return ANGLEConfig::BackendType::D3D11;
#elif defined(__APPLE__)
    return ANGLEConfig::BackendType::Metal;
#elif defined(__ANDROID__)
    return ANGLEConfig::BackendType::OpenGLES;
#else
    return ANGLEConfig::BackendType::OpenGL;
#endif
}

bool ANGLEContext::InitializeEGL() {
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_display == EGL_NO_DISPLAY) {
        return false;
    }
    
    EGLint major, minor;
    if (eglInitialize(m_display, &major, &minor) != EGL_TRUE) {
        return false;
    }
    
    return true;
}

bool ANGLEContext::CreateContext() {
    EGLint configAttribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };
    
    EGLint numConfigs;
    EGLConfig config;
    if (eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs) != EGL_TRUE || numConfigs == 0) {
        return false;
    }
    
    m_config = config;
    
    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 0,
        EGL_NONE
    };
    
    m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttribs);
    return m_context != EGL_NO_CONTEXT;
}

bool ANGLEContext::CreateSurface(Window* window) {
    if (!window) {
        return false;
    }

    // Android/iOS: Manually create surface using platform-specific handle
    WindowHandle* handle = window->GetNativeHandle();
    if (!handle || !handle->window) {
        return false;
    }
    
    EGLint surfaceAttribs[] = { EGL_NONE };
    m_surface = eglCreateWindowSurface(m_display, static_cast<EGLConfig>(m_config), 
                                      static_cast<EGLNativeWindowType>(handle->window), 
                                      surfaceAttribs);
    
    return m_surface != EGL_NO_SURFACE;
}

void ANGLEContext::ConfigureBackend(const ANGLEConfig& config) {
    // ANGLE automatically selects the appropriate backend for each platform
    // Windows: D3D11, macOS: Metal, Android: OpenGL ES
}

} // namespace SmartRenderer
