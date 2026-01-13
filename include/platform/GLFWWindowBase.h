#pragma once

#include "platform/Window.h"
#include <string>

struct GLFWwindow;

namespace SmartRenderer {

// Base class for GLFW-based windows (macOS and Windows)
// Contains all common GLFW window implementation
class GLFWWindowBase : public Window {
public:
    GLFWWindowBase();
    virtual ~GLFWWindowBase();

    bool Create(const WindowConfig& config) override;
    void Destroy() override;
    bool IsValid() const override;

    void SetTitle(const std::string& title) override;
    std::string GetTitle() const override;

    void SetSize(int width, int height) override;
    void GetSize(int& width, int& height) const override;

    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;

    void Show() override;
    void Hide() override;
    bool IsVisible() const override;

    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    bool IsMinimized() const override;
    bool IsMaximized() const override;

    void SetFullscreen(bool fullscreen) override;
    bool IsFullscreen() const override;

    void PollEvents() override;
    bool ShouldClose() const override;

    void SetResizeCallback(WindowResizeCallback callback) override;
    void SetCloseCallback(WindowCloseCallback callback) override;
    void SetFocusCallback(WindowFocusCallback callback) override;

    WindowHandle* GetNativeHandle() const override;

    void GetFramebufferSize(int& width, int& height) const override;
    float GetContentScaleX() const override;
    float GetContentScaleY() const override;

    void SwapBuffers() override;

protected:
    GLFWwindow* m_window;
    WindowHandle m_handle;
    bool m_shouldClose;
    bool m_visible;
    WindowConfig m_config;
    WindowResizeCallback m_resizeCallback;
    WindowCloseCallback m_closeCallback;
    WindowFocusCallback m_focusCallback;

    // Static callbacks for GLFW (to be implemented by derived classes)
    static void OnWindowResize(GLFWwindow* window, int width, int height);
    static void OnWindowClose(GLFWwindow* window);
    static void OnWindowFocus(GLFWwindow* window, int focused);
    
    // Helper to get window instance from GLFW window pointer
    static GLFWWindowBase* GetWindowInstance(GLFWwindow* window);
};

} // namespace SmartRenderer
