#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "platform/Window.h"
#include <windows.h>

// Undefine Windows macros that conflict with our method names
#ifdef CreateWindow
#undef CreateWindow
#endif
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#ifdef GetTickCount
#undef GetTickCount
#endif
#ifdef DeleteFile
#undef DeleteFile
#endif
#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace SmartRenderer {

struct WindowHandle {
    HWND hwnd;
    HDC hdc;
};

class WindowsWindow : public Window {
public:
    WindowsWindow();
    ~WindowsWindow() override;

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

private:
    HWND m_hwnd;
    HDC m_hdc;
    bool m_shouldClose;
    WindowConfig m_config;
    WindowResizeCallback m_resizeCallback;
    WindowCloseCallback m_closeCallback;
    WindowFocusCallback m_focusCallback;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static WindowsWindow* GetWindowFromHWND(HWND hwnd);
};

} // namespace SmartRenderer
