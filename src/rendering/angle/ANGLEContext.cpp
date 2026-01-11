#include "rendering/angle/ANGLEContext.h"
#include "platform/Window.h"
#include <stdexcept>

#ifdef USE_ANGLE
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#endif

namespace SmartRenderer {

ANGLEContext::ANGLEContext()
    : m_display(EGL_NO_DISPLAY)
    , m_context(EGL_NO_CONTEXT)
    , m_surface(EGL_NO_SURFACE)
    , m_config(nullptr)
    , m_initialized(false) {
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

    // Configure backend
    ConfigureBackend(config);

    // Initialize EGL
    if (!InitializeEGL()) {
        return false;
    }

    // Create context
    if (!CreateContext()) {
        return false;
    }

    // Create surface
    if (!CreateSurface(window)) {
        return false;
    }

    // Mark as initialized before MakeCurrent
    m_initialized = true;

    // Make context current
    if (!MakeCurrent()) {
        m_initialized = false;
        return false;
    }

    return true;
}

void ANGLEContext::Shutdown() {
    if (!m_initialized) {
        return;
    }

#ifdef USE_ANGLE
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    
    if (m_context != EGL_NO_CONTEXT) {
        eglDestroyContext(m_display, m_context);
    }
    
    if (m_surface != EGL_NO_SURFACE) {
        eglDestroySurface(m_display, m_surface);
    }
    
    if (m_display != EGL_NO_DISPLAY) {
        eglTerminate(m_display);
    }
#endif

    m_display = EGL_NO_DISPLAY;
    m_context = EGL_NO_CONTEXT;
    m_surface = EGL_NO_SURFACE;
    m_config = nullptr;
    m_initialized = false;
}

bool ANGLEContext::MakeCurrent() {
    if (!m_initialized) {
        #ifdef _DEBUG
        fprintf(stderr, "MakeCurrent: not initialized\n");
        #endif
        return false;
    }

#ifdef USE_ANGLE
    EGLBoolean result = eglMakeCurrent(m_display, m_surface, m_surface, m_context);
    if (result != EGL_TRUE) {
        #ifdef _DEBUG
        fprintf(stderr, "eglMakeCurrent failed\n");
        #endif
        return false;
    }
    
    #ifdef _DEBUG
    fprintf(stderr, "eglMakeCurrent succeeded\n");
    #endif
    return true;
#else
    return true;
#endif
}

void ANGLEContext::SwapBuffers() {
    if (!m_initialized) {
        return;
    }

#ifdef USE_ANGLE
    eglSwapBuffers(m_display, m_surface);
#endif
}

void ANGLEContext::SetSwapInterval(int interval) {
    if (!m_initialized) {
        return;
    }

#ifdef USE_ANGLE
    eglSwapInterval(m_display, interval);
#endif
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
#ifdef USE_ANGLE
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_display == EGL_NO_DISPLAY) {
        #ifdef _DEBUG
        fprintf(stderr, "eglGetDisplay failed\n");
        #endif
        return false;
    }
    
    EGLint major, minor;
    if (eglInitialize(m_display, &major, &minor) != EGL_TRUE) {
        #ifdef _DEBUG
        fprintf(stderr, "eglInitialize failed\n");
        #endif
        return false;
    }
    
    #ifdef _DEBUG
    fprintf(stderr, "EGL initialized: version %d.%d\n", major, minor);
    #endif
    
    return true;
#else
    // Stub implementation
    m_display = reinterpret_cast<EGLDisplay>(0x1); // Placeholder
    return true;
#endif
}

bool ANGLEContext::CreateContext() {
#ifdef USE_ANGLE
    // Choose config
    EGLint configAttribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };
    
    EGLint numConfigs;
    EGLConfig config;
    if (eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs) != EGL_TRUE || numConfigs == 0) {
        #ifdef _DEBUG
        fprintf(stderr, "eglChooseConfig failed, numConfigs=%d\n", numConfigs);
        #endif
        return false;
    }
    
    m_config = config;
    
    // Create context
    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 0,
        EGL_NONE
    };
    
    m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttribs);
    if (m_context == EGL_NO_CONTEXT) {
        #ifdef _DEBUG
        fprintf(stderr, "eglCreateContext failed\n");
        #endif
        return false;
    }
    
    #ifdef _DEBUG
    fprintf(stderr, "EGL context created successfully\n");
    #endif
    
    return true;
#else
    // Stub implementation
    m_context = reinterpret_cast<EGLContext>(0x1); // Placeholder
    return true;
#endif
}

bool ANGLEContext::CreateSurface(Window* window) {
    if (!window) {
        #ifdef _DEBUG
        fprintf(stderr, "CreateSurface: window is null\n");
        #endif
        return false;
    }

#ifdef USE_ANGLE
    void* handle = window->GetNativeHandle();
    if (!handle) {
        #ifdef _DEBUG
        fprintf(stderr, "CreateSurface: GetNativeHandle returned null\n");
        #endif
        return false;
    }
    
    EGLint surfaceAttribs[] = { EGL_NONE };
    
    // On Windows, WindowHandle is a struct with hwnd member
    #ifdef _WIN32
    // Cast to WindowHandle pointer and access hwnd
    struct WindowHandle { void* hwnd; void* hdc; };
    WindowHandle* winHandle = static_cast<WindowHandle*>(handle);
    
    #ifdef _DEBUG
    fprintf(stderr, "Creating EGL surface for HWND: %p\n", winHandle->hwnd);
    #endif
    
    m_surface = eglCreateWindowSurface(m_display, static_cast<EGLConfig>(m_config), 
                                      static_cast<EGLNativeWindowType>(winHandle->hwnd), 
                                      surfaceAttribs);
    #else
    m_surface = eglCreateWindowSurface(m_display, static_cast<EGLConfig>(m_config), 
                                      static_cast<EGLNativeWindowType>(handle), 
                                      surfaceAttribs);
    #endif
    
    if (m_surface == EGL_NO_SURFACE) {
        #ifdef _DEBUG
        fprintf(stderr, "eglCreateWindowSurface failed\n");
        #endif
        return false;
    }
    
    #ifdef _DEBUG
    fprintf(stderr, "EGL surface created successfully\n");
    #endif
    
    return true;
#else
    // Stub implementation
    m_surface = reinterpret_cast<EGLSurface>(0x1); // Placeholder
    return true;
#endif
}

void ANGLEContext::ConfigureBackend(const ANGLEConfig& config) {
    // In real implementation, would set EGL attributes based on backend type:
    // EGLint attribs[] = {
    //     EGL_PLATFORM_ANGLE_TYPE_ANGLE, ...,
    //     EGL_NONE
    // };
    // eglSetAttribute(EGL_PLATFORM_ANGLE_TYPE_ANGLE, ...);
}

} // namespace SmartRenderer
