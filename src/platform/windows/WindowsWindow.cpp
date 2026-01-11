#include "platform/windows/WindowsWindow.h"
#include <stdexcept>

namespace SmartRenderer {

WindowsWindow::WindowsWindow()
    : m_hwnd(nullptr)
    , m_hdc(nullptr)
    , m_shouldClose(false) {
}

WindowsWindow::~WindowsWindow() {
    Destroy();
}

bool WindowsWindow::Create(const WindowConfig& config) {
    m_config = config;

    // Register window class
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "SmartRendererWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassExA(&wc)) {
        return false;
    }

    // Calculate window size
    RECT rect = { 0, 0, config.width, config.height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    int x = config.x == -1 ? CW_USEDEFAULT : config.x;
    int y = config.y == -1 ? CW_USEDEFAULT : config.y;

    // Create window
    m_hwnd = CreateWindowExA(
        0,
        "SmartRendererWindow",
        config.title.c_str(),
        WS_OVERLAPPEDWINDOW,
        x, y, windowWidth, windowHeight,
        nullptr, nullptr, GetModuleHandle(nullptr), this
    );

    if (!m_hwnd) {
        return false;
    }

    m_hdc = GetDC(m_hwnd);
    SetWindowLongPtrA(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    return true;
}

void WindowsWindow::Destroy() {
    if (m_hwnd) {
        if (m_hdc) {
            ReleaseDC(m_hwnd, m_hdc);
            m_hdc = nullptr;
        }
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool WindowsWindow::IsValid() const {
    return m_hwnd != nullptr;
}

void WindowsWindow::SetTitle(const std::string& title) {
    m_config.title = title;
    if (m_hwnd) {
        SetWindowTextA(m_hwnd, title.c_str());
    }
}

std::string WindowsWindow::GetTitle() const {
    return m_config.title;
}

void WindowsWindow::SetSize(int width, int height) {
    m_config.width = width;
    m_config.height = height;
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void WindowsWindow::GetSize(int& width, int& height) const {
    width = m_config.width;
    height = m_config.height;
}

void WindowsWindow::SetPosition(int x, int y) {
    m_config.x = x;
    m_config.y = y;
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void WindowsWindow::GetPosition(int& x, int& y) const {
    x = m_config.x;
    y = m_config.y;
}

void WindowsWindow::Show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        BringWindowToTop(m_hwnd);
        SetForegroundWindow(m_hwnd);
    }
}

void WindowsWindow::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

bool WindowsWindow::IsVisible() const {
    return m_hwnd && IsWindowVisible(m_hwnd);
}

void WindowsWindow::Minimize() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_MINIMIZE);
    }
}

void WindowsWindow::Maximize() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_MAXIMIZE);
    }
}

void WindowsWindow::Restore() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_RESTORE);
    }
}

bool WindowsWindow::IsMinimized() const {
    return m_hwnd && IsIconic(m_hwnd);
}

bool WindowsWindow::IsMaximized() const {
    return m_hwnd && IsZoomed(m_hwnd);
}

void WindowsWindow::SetFullscreen(bool fullscreen) {
    m_config.fullscreen = fullscreen;
    // Implementation would set fullscreen mode
}

bool WindowsWindow::IsFullscreen() const {
    return m_config.fullscreen;
}

void WindowsWindow::PollEvents() {
    MSG msg;
    while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool WindowsWindow::ShouldClose() const {
    return m_shouldClose;
}

void WindowsWindow::SetResizeCallback(WindowResizeCallback callback) {
    m_resizeCallback = callback;
}

void WindowsWindow::SetCloseCallback(WindowCloseCallback callback) {
    m_closeCallback = callback;
}

void WindowsWindow::SetFocusCallback(WindowFocusCallback callback) {
    m_focusCallback = callback;
}

WindowHandle* WindowsWindow::GetNativeHandle() const {
    static WindowHandle handle;
    handle.hwnd = m_hwnd;
    handle.hdc = m_hdc;
    return &handle;
}

void WindowsWindow::GetFramebufferSize(int& width, int& height) const {
    RECT rect;
    if (m_hwnd && GetClientRect(m_hwnd, &rect)) {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    } else {
        width = m_config.width;
        height = m_config.height;
    }
}

float WindowsWindow::GetContentScaleX() const {
    // DPI scaling
    if (m_hwnd) {
        HDC hdc = GetDC(m_hwnd);
        int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(m_hwnd, hdc);
        return dpi / 96.0f;
    }
    return 1.0f;
}

float WindowsWindow::GetContentScaleY() const {
    return GetContentScaleX();
}

void WindowsWindow::SwapBuffers() {
    // Would call eglSwapBuffers or similar
}

LRESULT CALLBACK WindowsWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowsWindow* window = GetWindowFromHWND(hwnd);
    if (!window) {
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }

    // Forward input messages to platform's input manager
    // In real implementation, would get input manager from platform and forward messages

    switch (uMsg) {
    case WM_SIZE:
        if (window->m_resizeCallback) {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            window->m_config.width = width;
            window->m_config.height = height;
            window->m_resizeCallback(width, height);
        }
        return 0;

    case WM_CLOSE:
        window->m_shouldClose = true;
        if (window->m_closeCallback) {
            window->m_closeCallback();
        }
        return 0;

    case WM_SETFOCUS:
        if (window->m_focusCallback) {
            window->m_focusCallback(true);
        }
        return 0;

    case WM_KILLFOCUS:
        if (window->m_focusCallback) {
            window->m_focusCallback(false);
        }
        return 0;

    default:
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

WindowsWindow* WindowsWindow::GetWindowFromHWND(HWND hwnd) {
    return reinterpret_cast<WindowsWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
}

} // namespace SmartRenderer
