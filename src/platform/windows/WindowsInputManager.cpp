#include "platform/windows/WindowsInputManager.h"
#include "platform/InputManager.h"
#include "math/Vector.h"
#include <windows.h>
#include <map>
#include <unordered_set>
#include <memory>

namespace SmartRenderer {

class WindowsInputManager::Impl {
public:
    Impl();
    ~Impl();

    bool IsKeyPressed(KeyCode key) const;
    bool IsKeyReleased(KeyCode key) const;
    void SetKeyCallback(KeyCallback callback);

    bool IsMouseButtonPressed(MouseButton button) const;
    Vector2 GetMousePosition() const;
    Vector2 GetMouseDelta() const;
    void SetMouseButtonCallback(MouseButtonCallback callback);
    void SetMouseMoveCallback(MouseMoveCallback callback);
    void SetScrollCallback(ScrollCallback callback);

    void Update();

    // Windows message handler
    void HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    std::unordered_set<KeyCode> m_pressedKeys;
    std::unordered_set<KeyCode> m_releasedKeys;
    std::unordered_set<MouseButton> m_pressedButtons;
    Vector2 m_mousePosition;
    Vector2 m_mouseDelta;
    Vector2 m_lastMousePosition;

    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    ScrollCallback m_scrollCallback;

    KeyCode ConvertKeyCode(WPARAM wParam);
    MouseButton ConvertMouseButton(WPARAM wParam);
};

WindowsInputManager::WindowsInputManager()
    : m_impl(std::make_unique<Impl>()) {
}

WindowsInputManager::~WindowsInputManager() {
}

bool WindowsInputManager::IsKeyPressed(KeyCode key) const {
    return m_impl->IsKeyPressed(key);
}

bool WindowsInputManager::IsKeyReleased(KeyCode key) const {
    return m_impl->IsKeyReleased(key);
}

void WindowsInputManager::SetKeyCallback(KeyCallback callback) {
    m_impl->SetKeyCallback(callback);
}

bool WindowsInputManager::IsMouseButtonPressed(MouseButton button) const {
    return m_impl->IsMouseButtonPressed(button);
}

Vector2 WindowsInputManager::GetMousePosition() const {
    return m_impl->GetMousePosition();
}

Vector2 WindowsInputManager::GetMouseDelta() const {
    return m_impl->GetMouseDelta();
}

void WindowsInputManager::SetMouseButtonCallback(MouseButtonCallback callback) {
    m_impl->SetMouseButtonCallback(callback);
}

void WindowsInputManager::SetMouseMoveCallback(MouseMoveCallback callback) {
    m_impl->SetMouseMoveCallback(callback);
}

void WindowsInputManager::SetScrollCallback(ScrollCallback callback) {
    m_impl->SetScrollCallback(callback);
}

void WindowsInputManager::Update() {
    m_impl->Update();
}

void WindowsInputManager::HandleMessage(unsigned int msg, unsigned long long wParam, long long lParam) {
    m_impl->HandleMessage(msg, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));
}

// Impl implementation
WindowsInputManager::Impl::Impl()
    : m_mousePosition(Vector2::Zero)
    , m_mouseDelta(Vector2::Zero)
    , m_lastMousePosition(Vector2::Zero) {
}

WindowsInputManager::Impl::~Impl() {
}

bool WindowsInputManager::Impl::IsKeyPressed(KeyCode key) const {
    return m_pressedKeys.find(key) != m_pressedKeys.end();
}

bool WindowsInputManager::Impl::IsKeyReleased(KeyCode key) const {
    return m_releasedKeys.find(key) != m_releasedKeys.end();
}

void WindowsInputManager::Impl::SetKeyCallback(KeyCallback callback) {
    m_keyCallback = callback;
}

bool WindowsInputManager::Impl::IsMouseButtonPressed(MouseButton button) const {
    return m_pressedButtons.find(button) != m_pressedButtons.end();
}

Vector2 WindowsInputManager::Impl::GetMousePosition() const {
    return m_mousePosition;
}

Vector2 WindowsInputManager::Impl::GetMouseDelta() const {
    return m_mouseDelta;
}

void WindowsInputManager::Impl::SetMouseButtonCallback(MouseButtonCallback callback) {
    m_mouseButtonCallback = callback;
}

void WindowsInputManager::Impl::SetMouseMoveCallback(MouseMoveCallback callback) {
    m_mouseMoveCallback = callback;
}

void WindowsInputManager::Impl::SetScrollCallback(ScrollCallback callback) {
    m_scrollCallback = callback;
}

void WindowsInputManager::Impl::Update() {
    m_releasedKeys.clear();
    m_mouseDelta = Vector2::Zero;
    m_lastMousePosition = m_mousePosition;
}

void WindowsInputManager::Impl::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        KeyCode key = ConvertKeyCode(wParam);
        if (m_pressedKeys.find(key) == m_pressedKeys.end()) {
            m_pressedKeys.insert(key);
            if (m_keyCallback) {
                m_keyCallback(key, InputAction::Press);
            }
        }
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        KeyCode key = ConvertKeyCode(wParam);
        m_pressedKeys.erase(key);
        m_releasedKeys.insert(key);
        if (m_keyCallback) {
            m_keyCallback(key, InputAction::Release);
        }
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN: {
        MouseButton button = ConvertMouseButton(msg);
        m_pressedButtons.insert(button);
        if (m_mouseButtonCallback) {
            m_mouseButtonCallback(button, InputAction::Press);
        }
        break;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP: {
        MouseButton button = ConvertMouseButton(msg);
        m_pressedButtons.erase(button);
        if (m_mouseButtonCallback) {
            m_mouseButtonCallback(button, InputAction::Release);
        }
        break;
    }
    case WM_MOUSEMOVE: {
        float x = static_cast<float>(LOWORD(lParam));
        float y = static_cast<float>(HIWORD(lParam));
        m_mousePosition = Vector2(x, y);
        m_mouseDelta = m_mousePosition - m_lastMousePosition;
        if (m_mouseMoveCallback) {
            m_mouseMoveCallback(x, y);
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
        if (m_scrollCallback) {
            m_scrollCallback(0.0f, delta);
        }
        break;
    }
    }
}

KeyCode WindowsInputManager::Impl::ConvertKeyCode(WPARAM wParam) {
    if (wParam >= 'A' && wParam <= 'Z') {
        return static_cast<KeyCode>(wParam);
    }
    if (wParam >= '0' && wParam <= '9') {
        return static_cast<KeyCode>(wParam);
    }
    switch (wParam) {
    case VK_SPACE: return KeyCode::Space;
    case VK_ESCAPE: return KeyCode::Escape;
    case VK_RETURN: return KeyCode::Enter;
    case VK_TAB: return KeyCode::Tab;
    case VK_BACK: return KeyCode::Backspace;
    case VK_LEFT: return KeyCode::Left;
    case VK_RIGHT: return KeyCode::Right;
    case VK_UP: return KeyCode::Up;
    case VK_DOWN: return KeyCode::Down;
    case VK_SHIFT: return KeyCode::LeftShift;
    case VK_CONTROL: return KeyCode::LeftControl;
    case VK_MENU: return KeyCode::LeftAlt;
    default: return KeyCode::Unknown;
    }
}

MouseButton WindowsInputManager::Impl::ConvertMouseButton(WPARAM wParam) {
    switch (wParam) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return MouseButton::Left;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return MouseButton::Right;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return MouseButton::Middle;
    default:
        return MouseButton::Left;
    }
}

// Touch support (Windows desktop doesn't typically support touch in this way, so we provide stubs)
int WindowsInputManager::GetTouchCount() const {
    return 0; // Desktop Windows typically doesn't have touch input, return 0
}

Vector2 WindowsInputManager::GetTouchPosition(int index) const {
    (void)index; // Unused
    return Vector2::Zero; // No touch support on desktop
}

void WindowsInputManager::SetTouchCallback(TouchCallback callback) {
    (void)callback; // Unused - no touch support on desktop Windows
    // Touch callbacks are not supported on desktop Windows in this implementation
}

} // namespace SmartRenderer
