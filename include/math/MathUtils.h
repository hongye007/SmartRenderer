#pragma once

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"

namespace SmartRenderer {
namespace Math {

// Utility functions
float Clamp(float value, float min, float max);
float Lerp(float a, float b, float t);
float SmoothStep(float edge0, float edge1, float x);
float SmootherStep(float edge0, float edge1, float x);

Vector2 Clamp(const Vector2& value, const Vector2& min, const Vector2& max);
Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max);
Vector4 Clamp(const Vector4& value, const Vector4& min, const Vector4& max);

Vector2 Lerp(const Vector2& a, const Vector2& b, float t);
Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
Vector4 Lerp(const Vector4& a, const Vector4& b, float t);

Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
Matrix4 Lerp(const Matrix4& a, const Matrix4& b, float t);

float DegreesToRadians(float degrees);
float RadiansToDegrees(float radians);

} // namespace Math
} // namespace SmartRenderer
