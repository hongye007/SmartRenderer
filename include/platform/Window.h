#pragma once

#include <string>
#include <functional>

namespace SmartRenderer {

// Forward declarations
struct WindowHandle;

// Window event callbacks
using WindowResizeCallback = std::function<void(int width, int height)>;
using WindowCloseCallback = std::function<void()>;
using WindowFocusCallback = std::function<void(bool focused)>;

// Window configuration
struct WindowConfig {
    std::string title = "SmartRenderer Window";
    int width = 800;
    int height = 600;
    int x = -1; // -1 for centered
    int y = -1; // -1 for centered
    bool resizable = true;
    bool fullscreen = false;
    bool borderless = false;
    bool vsync = true;
    int multisample = 0; // 0 = no multisampling
};

// Abstract window interface
class Window {
public:
    virtual ~Window() = default;

    // Window lifecycle
    virtual bool Create(const WindowConfig& config) = 0;
    virtual void Destroy() = 0;
    virtual bool IsValid() const = 0;

    // Window properties
    virtual void SetTitle(const std::string& title) = 0;
    virtual std::string GetTitle() const = 0;

    virtual void SetSize(int width, int height) = 0;
    virtual void GetSize(int& width, int& height) const = 0;

    virtual void SetPosition(int x, int y) = 0;
    virtual void GetPosition(int& x, int& y) const = 0;

    // Window state
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual bool IsVisible() const = 0;

    virtual void Minimize() = 0;
    virtual void Maximize() = 0;
    virtual void Restore() = 0;
    virtual bool IsMinimized() const = 0;
    virtual bool IsMaximized() const = 0;

    virtual void SetFullscreen(bool fullscreen) = 0;
    virtual bool IsFullscreen() const = 0;

    // Event handling
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;

    // Callbacks
    virtual void SetResizeCallback(WindowResizeCallback callback) = 0;
    virtual void SetCloseCallback(WindowCloseCallback callback) = 0;
    virtual void SetFocusCallback(WindowFocusCallback callback) = 0;

    // Platform-specific handle (for rendering backends)
    virtual WindowHandle* GetNativeHandle() const = 0;

    // Framebuffer information (for rendering)
    virtual void GetFramebufferSize(int& width, int& height) const = 0;
    virtual float GetContentScaleX() const = 0;
    virtual float GetContentScaleY() const = 0;

    // Swap buffers (if needed by platform)
    virtual void SwapBuffers() = 0;
};

} // namespace SmartRenderer
