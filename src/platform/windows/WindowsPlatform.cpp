#include "platform/windows/WindowsPlatform.h"
#include "platform/windows/WindowsWindow.h"
#include "platform/windows/WindowsInputManager.h"
#include "platform/windows/WindowsFileSystem.h"
#include "platform/Window.h"
#include <windows.h>
#include <shlobj.h>
#include <chrono>

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

namespace SmartRenderer {

WindowsPlatform::WindowsPlatform()
    : m_initialized(false)
    , m_startTime(0.0) {
}

WindowsPlatform::~WindowsPlatform() {
    Shutdown();
}

bool WindowsPlatform::Initialize(const PlatformConfig& config) {
    if (m_initialized) {
        return true;
    }

    m_config = config;
    
    // Initialize time
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    m_startTime = std::chrono::duration<double>(duration).count();

    // Initialize input manager
    m_inputManager = std::make_unique<WindowsInputManager>();

    // Initialize file system
    m_fileSystem = std::make_unique<WindowsFileSystem>();

    m_initialized = true;
    return true;
}

void WindowsPlatform::Shutdown() {
    if (!m_initialized) {
        return;
    }

    m_inputManager.reset();
    m_fileSystem.reset();
    m_initialized = false;
}

void WindowsPlatform::RunMainLoop() {
    MSG msg;
    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Application update would go here
    }
}

void WindowsPlatform::RequestExit() {
    PostQuitMessage(0);
}

Window* WindowsPlatform::CreateWindow(int width, int height, const std::string& title) {
    auto window = new WindowsWindow();
    WindowConfig config;
    config.width = width;
    config.height = height;
    config.title = title;
    if (window->Create(config)) {
        return window;
    }
    delete window;
    return nullptr;
}

void WindowsPlatform::DestroyWindow(Window* window) {
    if (window) {
        window->Destroy();
        delete window;
    }
}

InputManager* WindowsPlatform::GetInputManager() {
    return m_inputManager.get();
}

FileSystem* WindowsPlatform::GetFileSystem() {
    return m_fileSystem.get();
}

int WindowsPlatform::GetScreenWidth() const {
    return GetSystemMetrics(SM_CXSCREEN);
}

int WindowsPlatform::GetScreenHeight() const {
    return GetSystemMetrics(SM_CYSCREEN);
}

float WindowsPlatform::GetScreenDensity() const {
    // Windows DPI scaling
    HDC hdc = GetDC(nullptr);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(nullptr, hdc);
    return dpi / 96.0f; // 96 DPI is standard
}

std::string WindowsPlatform::GetResourcePath(const std::string& relativePath) const {
    // In real implementation, would get executable directory
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exePath = exePath.substr(0, lastSlash + 1);
    }
    return exePath + relativePath;
}

std::string WindowsPlatform::GetWritablePath(const std::string& relativePath) const {
    // In real implementation, would use AppData directory
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, buffer))) {
        return std::string(buffer) + "\\" + m_config.appName + "\\" + relativePath;
    }
    return relativePath;
}

double WindowsPlatform::GetCurrentTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

double WindowsPlatform::GetTimeSinceStart() const {
    return GetCurrentTime() - m_startTime;
}

} // namespace SmartRenderer
