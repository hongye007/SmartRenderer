#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>

namespace SmartRenderer {

// Constants
static constexpr float PI = 3.14159265358979323846f;
static constexpr float DEG_TO_RAD = PI / 180.0f;
static constexpr float RAD_TO_DEG = 180.0f / PI;
static constexpr float EPSILON = 1e-6f;

// Forward declarations
struct Vector2;
struct Vector3;
struct Vector4;
struct Quaternion;
class Matrix4;
struct Color;

// Color representation
struct Color {
    float r, g, b, a;

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Transparent;
};

} // namespace SmartRenderer
