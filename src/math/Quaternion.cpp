#include "math/Quaternion.h"
#include "math/Matrix.h"
#include <cmath>

namespace SmartRenderer {

const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion Quaternion::FromEulerAngles(float pitch, float yaw, float roll) {
    float halfPitch = pitch * 0.5f;
    float halfYaw = yaw * 0.5f;
    float halfRoll = roll * 0.5f;

    float sp = std::sin(halfPitch);
    float cp = std::cos(halfPitch);
    float sy = std::sin(halfYaw);
    float cy = std::cos(halfYaw);
    float sr = std::sin(halfRoll);
    float cr = std::cos(halfRoll);

    Quaternion result;
    result.x = cr * sp * cy - sr * cp * sy;
    result.y = cr * cp * sy + sr * sp * cy;
    result.z = sr * cp * cy - cr * sp * sy;
    result.w = cr * cp * cy + sr * sp * sy;

    return result;
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle) {
    float halfAngle = angle * 0.5f;
    float s = std::sin(halfAngle);
    Vector3 normalizedAxis = axis.Normalized();

    Quaternion result;
    result.x = normalizedAxis.x * s;
    result.y = normalizedAxis.y * s;
    result.z = normalizedAxis.z * s;
    result.w = std::cos(halfAngle);

    return result;
}

Quaternion Quaternion::FromRotationMatrix(const Matrix4& matrix) {
    float trace = matrix.m[0] + matrix.m[5] + matrix.m[10];
    Quaternion result;

    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 2.0f; // s = 4 * qw
        float invS = 1.0f / s;
        result.w = s * 0.25f;
        result.x = (matrix.m[9] - matrix.m[6]) * invS;
        result.y = (matrix.m[2] - matrix.m[8]) * invS;
        result.z = (matrix.m[4] - matrix.m[1]) * invS;
    } else if (matrix.m[0] > matrix.m[5] && matrix.m[0] > matrix.m[10]) {
        float s = std::sqrt(1.0f + matrix.m[0] - matrix.m[5] - matrix.m[10]) * 2.0f;
        float invS = 1.0f / s;
        result.w = (matrix.m[9] - matrix.m[6]) * invS;
        result.x = s * 0.25f;
        result.y = (matrix.m[1] + matrix.m[4]) * invS;
        result.z = (matrix.m[2] + matrix.m[8]) * invS;
    } else if (matrix.m[5] > matrix.m[10]) {
        float s = std::sqrt(1.0f + matrix.m[5] - matrix.m[0] - matrix.m[10]) * 2.0f;
        float invS = 1.0f / s;
        result.w = (matrix.m[2] - matrix.m[8]) * invS;
        result.x = (matrix.m[1] + matrix.m[4]) * invS;
        result.y = s * 0.25f;
        result.z = (matrix.m[6] + matrix.m[9]) * invS;
    } else {
        float s = std::sqrt(1.0f + matrix.m[10] - matrix.m[0] - matrix.m[5]) * 2.0f;
        float invS = 1.0f / s;
        result.w = (matrix.m[4] - matrix.m[1]) * invS;
        result.x = (matrix.m[2] + matrix.m[8]) * invS;
        result.y = (matrix.m[6] + matrix.m[9]) * invS;
        result.z = s * 0.25f;
    }

    return result.Normalized();
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    Quaternion result;
    result.w = w * other.w - x * other.x - y * other.y - z * other.z;
    result.x = w * other.x + x * other.w + y * other.z - z * other.y;
    result.y = w * other.y - x * other.z + y * other.w + z * other.x;
    result.z = w * other.z + x * other.y - y * other.x + z * other.w;
    return result;
}

Vector3 Quaternion::operator*(const Vector3& v) const {
    Quaternion qv(0.0f, v.x, v.y, v.z);
    Quaternion result = (*this) * qv * Conjugate();
    return Vector3(result.x, result.y, result.z);
}

void Quaternion::Normalize() {
    float len = Length();
    if (len > EPSILON) {
        float invLen = 1.0f / len;
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }
}

Quaternion Quaternion::Normalized() const {
    Quaternion result = *this;
    result.Normalize();
    return result;
}

Quaternion Quaternion::Conjugate() const {
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inverse() const {
    float lenSq = LengthSquared();
    if (lenSq > EPSILON) {
        float invLenSq = 1.0f / lenSq;
        return Quaternion(-x * invLenSq, -y * invLenSq, -z * invLenSq, w * invLenSq);
    }
    return Identity;
}

float Quaternion::Length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Quaternion::LengthSquared() const {
    return x * x + y * y + z * z + w * w;
}

Vector3 Quaternion::ToEulerAngles() const {
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    float roll = std::atan2(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (w * y - z * x);
    float pitch;
    if (std::abs(sinp) >= 1.0f) {
        pitch = std::copysign(PI / 2.0f, sinp);
    } else {
        pitch = std::asin(sinp);
    }

    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    float yaw = std::atan2(siny_cosp, cosy_cosp);

    return Vector3(pitch, yaw, roll);
}

Matrix4 Quaternion::ToRotationMatrix() const {
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    Matrix4 result;
    result.SetIdentity();
    result.m[0] = 1.0f - 2.0f * (yy + zz);
    result.m[1] = 2.0f * (xy + wz);
    result.m[2] = 2.0f * (xz - wy);
    result.m[4] = 2.0f * (xy - wz);
    result.m[5] = 1.0f - 2.0f * (xx + zz);
    result.m[6] = 2.0f * (yz + wx);
    result.m[8] = 2.0f * (xz + wy);
    result.m[9] = 2.0f * (yz - wx);
    result.m[10] = 1.0f - 2.0f * (xx + yy);

    return result;
}

} // namespace SmartRenderer
