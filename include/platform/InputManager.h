#pragma once

#include "math/Vector.h"
#include <functional>
#include <vector>

namespace SmartRenderer {

// Input event types
enum class KeyCode {
    Unknown = -1,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Key0 = 48,
    Key1 = 49,
    Key2 = 50,
    Key3 = 51,
    Key4 = 52,
    Key5 = 53,
    Key6 = 54,
    Key7 = 55,
    Key8 = 56,
    Key9 = 57,
    Semicolon = 59,
    Equal = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4
};

enum class InputAction {
    Release = 0,
    Press = 1,
    Repeat = 2
};

// Input callbacks
using KeyCallback = std::function<void(KeyCode key, InputAction action)>;
using MouseButtonCallback = std::function<void(MouseButton button, InputAction action)>;
using MouseMoveCallback = std::function<void(float x, float y)>;
using ScrollCallback = std::function<void(float xOffset, float yOffset)>;
using TouchCallback = std::function<void(int touchId, float x, float y, InputAction action)>;

// Input manager interface
class InputManager {
public:
    virtual ~InputManager() = default;

    // Keyboard input
    virtual bool IsKeyPressed(KeyCode key) const = 0;
    virtual bool IsKeyReleased(KeyCode key) const = 0;
    virtual void SetKeyCallback(KeyCallback callback) = 0;

    // Mouse input
    virtual bool IsMouseButtonPressed(MouseButton button) const = 0;
    virtual Vector2 GetMousePosition() const = 0;
    virtual Vector2 GetMouseDelta() const = 0;
    virtual void SetMouseButtonCallback(MouseButtonCallback callback) = 0;
    virtual void SetMouseMoveCallback(MouseMoveCallback callback) = 0;
    virtual void SetScrollCallback(ScrollCallback callback) = 0;

    // Touch input (for mobile platforms)
    virtual int GetTouchCount() const = 0;
    virtual Vector2 GetTouchPosition(int touchId) const = 0;
    virtual void SetTouchCallback(TouchCallback callback) = 0;

    // Update (called each frame)
    virtual void Update() = 0;
};

} // namespace SmartRenderer
