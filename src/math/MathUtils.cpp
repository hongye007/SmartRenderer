#include "math/MathUtils.h"
#include <algorithm>
#include <cmath>

namespace SmartRenderer {
namespace Math {

float Clamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float SmoothStep(float edge0, float edge1, float x) {
    float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float SmootherStep(float edge0, float edge1, float x) {
    float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

Vector2 Clamp(const Vector2& value, const Vector2& min, const Vector2& max) {
    return Vector2(
        std::max(min.x, std::min(max.x, value.x)),
        std::max(min.y, std::min(max.y, value.y))
    );
}

Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max) {
    return Vector3(
        std::max(min.x, std::min(max.x, value.x)),
        std::max(min.y, std::min(max.y, value.y)),
        std::max(min.z, std::min(max.z, value.z))
    );
}

Vector4 Clamp(const Vector4& value, const Vector4& min, const Vector4& max) {
    return Vector4(
        std::max(min.x, std::min(max.x, value.x)),
        std::max(min.y, std::min(max.y, value.y)),
        std::max(min.z, std::min(max.z, value.z)),
        std::max(min.w, std::min(max.w, value.w))
    );
}

Vector2 Lerp(const Vector2& a, const Vector2& b, float t) {
    return Vector2(
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t)
    );
}

Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
    return Vector3(
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t)
    );
}

Vector4 Lerp(const Vector4& a, const Vector4& b, float t) {
    return Vector4(
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t),
        Lerp(a.w, b.w, t)
    );
}

Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t) {
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    
    // If dot is negative, negate one quaternion to take shorter path
    Quaternion b2 = b;
    if (dot < 0.0f) {
        b2.x = -b2.x;
        b2.y = -b2.y;
        b2.z = -b2.z;
        b2.w = -b2.w;
        dot = -dot;
    }
    
    // If quaternions are very close, use linear interpolation
    if (dot > 0.9995f) {
        Quaternion result;
        result.x = Lerp(a.x, b2.x, t);
        result.y = Lerp(a.y, b2.y, t);
        result.z = Lerp(a.z, b2.z, t);
        result.w = Lerp(a.w, b2.w, t);
        return result.Normalized();
    }
    
    // Spherical interpolation
    float theta = std::acos(Clamp(dot, -1.0f, 1.0f));
    float sinTheta = std::sin(theta);
    float w1 = std::sin((1.0f - t) * theta) / sinTheta;
    float w2 = std::sin(t * theta) / sinTheta;
    
    Quaternion result;
    result.x = a.x * w1 + b2.x * w2;
    result.y = a.y * w1 + b2.y * w2;
    result.z = a.z * w1 + b2.z * w2;
    result.w = a.w * w1 + b2.w * w2;
    
    return result;
}

Matrix4 Lerp(const Matrix4& a, const Matrix4& b, float t) {
    Matrix4 result;
    for (int i = 0; i < 16; ++i) {
        result.m[i] = Lerp(a.m[i], b.m[i], t);
    }
    return result;
}

float DegreesToRadians(float degrees) {
    return degrees * DEG_TO_RAD;
}

float RadiansToDegrees(float radians) {
    return radians * RAD_TO_DEG;
}

} // namespace Math
} // namespace SmartRenderer
