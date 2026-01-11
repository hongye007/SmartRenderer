#pragma once

#include "MathTypes.h"
#include "Vector.h"
#include "Matrix.h"

namespace SmartRenderer {

// Forward declaration
class Matrix4;

// Quaternion
struct Quaternion {
    float x, y, z, w;

    Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    static const Quaternion Identity;
    
    static Quaternion FromEulerAngles(float pitch, float yaw, float roll);
    static Quaternion FromAxisAngle(const Vector3& axis, float angle);
    static Quaternion FromRotationMatrix(const Matrix4& matrix);

    Quaternion operator*(const Quaternion& other) const;
    Vector3 operator*(const Vector3& v) const;

    void Normalize();
    Quaternion Normalized() const;
    Quaternion Conjugate() const;
    Quaternion Inverse() const;

    float Length() const;
    float LengthSquared() const;

    Vector3 ToEulerAngles() const;
    Matrix4 ToRotationMatrix() const;
};

} // namespace SmartRenderer
