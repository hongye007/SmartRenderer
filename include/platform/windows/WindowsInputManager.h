#pragma once

#include "platform/InputManager.h"
#include <memory>

namespace SmartRenderer {

class WindowsInputManager : public InputManager {
public:
    WindowsInputManager();
    ~WindowsInputManager() override;

    bool IsKeyPressed(KeyCode key) const override;
    bool IsKeyReleased(KeyCode key) const override;
    void SetKeyCallback(KeyCallback callback) override;

    bool IsMouseButtonPressed(MouseButton button) const override;
    Vector2 GetMousePosition() const override;
    Vector2 GetMouseDelta() const override;
    void SetMouseButtonCallback(MouseButtonCallback callback) override;
    void SetMouseMoveCallback(MouseMoveCallback callback) override;
    void SetScrollCallback(ScrollCallback callback) override;

    int GetTouchCount() const override;
    Vector2 GetTouchPosition(int touchId) const override;
    void SetTouchCallback(TouchCallback callback) override;

    void Update() override;

    // Windows message handler
    void HandleMessage(unsigned int msg, unsigned long long wParam, long long lParam);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace SmartRenderer
