#pragma once

#include <memory>

// Forward declarations for EGL
typedef void* EGLDisplay;
typedef void* EGLContext;
typedef void* EGLSurface;
typedef void* EGLConfig;

#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_SURFACE ((EGLSurface)0)

namespace SmartRenderer {

class Window;

// ANGLE context configuration
struct ANGLEConfig {
    int majorVersion = 3;
    int minorVersion = 0;
    bool debug = false;
    bool enableValidation = false;
    
    // Backend selection
    enum class BackendType {
        Default,    // Let ANGLE choose
        D3D11,      // DirectX 11 (Windows)
        D3D12,      // DirectX 12 (Windows)
        Metal,      // Metal (macOS/iOS)
        Vulkan,     // Vulkan (Android/Windows)
        OpenGL,     // OpenGL (fallback)
        OpenGLES    // OpenGL ES (native)
    } backendType = BackendType::Default;
};

// ANGLE context manager
class ANGLEContext {
public:
    ANGLEContext();
    ~ANGLEContext();

    // Context lifecycle
    bool Initialize(Window* window, const ANGLEConfig& config);
    void Shutdown();

    // Context operations
    bool MakeCurrent();
    void SwapBuffers();
    void SetSwapInterval(int interval);

    // Getters
    EGLDisplay GetDisplay() const { return m_display; }
    EGLContext GetContext() const { return m_context; }
    EGLSurface GetSurface() const { return m_surface; }
    bool IsInitialized() const { return m_initialized; }

    // Platform-specific backend configuration
    static ANGLEConfig::BackendType GetDefaultBackend();

private:
    EGLDisplay m_display;
    EGLContext m_context;
    EGLSurface m_surface;
    EGLConfig m_config;
    bool m_initialized;
    bool m_usingGLFW;  // Track if using GLFW's EGL context

    bool InitializeEGL();
    bool CreateContext();
    bool CreateSurface(Window* window);
    void ConfigureBackend(const ANGLEConfig& config);
};

} // namespace SmartRenderer
