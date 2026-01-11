#pragma once

#include "MathTypes.h"
#include "Vector.h"
#include <cstring>

namespace SmartRenderer {

// Forward declaration
struct Quaternion;

// Matrix4x4
class Matrix4 {
public:
    float m[16];

    Matrix4();
    Matrix4(const float* data);
    Matrix4(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33);

    Matrix4 operator*(const Matrix4& other) const;
    Vector4 operator*(const Vector4& v) const;
    Vector3 operator*(const Vector3& v) const;

    Matrix4& operator*=(const Matrix4& other);

    void SetIdentity();
    void SetZero();

    Matrix4 Transpose() const;
    Matrix4 Inverse() const;
    float Determinant() const;

    // Static factory methods
    static Matrix4 CreateTranslation(const Vector3& translation);
    static Matrix4 CreateRotation(const Quaternion& rotation);
    static Matrix4 CreateRotationX(float angle);
    static Matrix4 CreateRotationY(float angle);
    static Matrix4 CreateRotationZ(float angle);
    static Matrix4 CreateScale(const Vector3& scale);
    static Matrix4 CreatePerspective(float fov, float aspect, float nearPlane, float farPlane);
    static Matrix4 CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static Matrix4 CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up);

    static const Matrix4 Identity;
    static const Matrix4 Zero;
};

} // namespace SmartRenderer
