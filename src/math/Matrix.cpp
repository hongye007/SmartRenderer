#include "math/Matrix.h"
#include "math/Quaternion.h"
#include <cmath>
#include <cstring>

namespace SmartRenderer {

Matrix4::Matrix4() {
    SetIdentity();
}

Matrix4::Matrix4(const float* data) {
    std::memcpy(m, data, 16 * sizeof(float));
}

Matrix4::Matrix4(float m00, float m01, float m02, float m03,
                 float m10, float m11, float m12, float m13,
                 float m20, float m21, float m22, float m23,
                 float m30, float m31, float m32, float m33) {
    m[0] = m00; m[1] = m01; m[2] = m02; m[3] = m03;
    m[4] = m10; m[5] = m11; m[6] = m12; m[7] = m13;
    m[8] = m20; m[9] = m21; m[10] = m22; m[11] = m23;
    m[12] = m30; m[13] = m31; m[14] = m32; m[15] = m33;
}

void Matrix4::SetIdentity() {
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void Matrix4::SetZero() {
    std::memset(m, 0, 16 * sizeof(float));
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i * 4 + j] += m[i * 4 + k] * other.m[k * 4 + j];
            }
        }
    }
    return result;
}

Vector4 Matrix4::operator*(const Vector4& v) const {
    return Vector4(
        m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
        m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
        m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
    );
}

Vector3 Matrix4::operator*(const Vector3& v) const {
    Vector4 result = (*this) * Vector4(v, 1.0f);
    return Vector3(result.x, result.y, result.z);
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
    *this = *this * other;
    return *this;
}

Matrix4 Matrix4::Transpose() const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] = m[j * 4 + i];
        }
    }
    return result;
}

float Matrix4::Determinant() const {
    float a = m[0], b = m[1], c = m[2], d = m[3];
    float e = m[4], f = m[5], g = m[6], h = m[7];
    float i = m[8], j = m[9], k = m[10], l = m[11];
    float mm = m[12], n = m[13], o = m[14], p = m[15];

    return a * (f * (k * p - l * o) - g * (j * p - l * n) + h * (j * o - k * n))
         - b * (e * (k * p - l * o) - g * (i * p - l * mm) + h * (i * o - k * mm))
         + c * (e * (j * p - l * n) - f * (i * p - l * mm) + h * (i * n - j * mm))
         - d * (e * (j * o - k * n) - f * (i * o - k * mm) + g * (i * n - j * mm));
}

Matrix4 Matrix4::Inverse() const {
    float det = Determinant();
    if (std::abs(det) < EPSILON) {
        return Identity; // Return identity if matrix is singular
    }

    Matrix4 result;
    float invDet = 1.0f / det;

    // Calculate inverse using adjugate matrix
    result.m[0] = (m[5] * (m[10] * m[15] - m[11] * m[14]) - m[6] * (m[9] * m[15] - m[11] * m[13]) + m[7] * (m[9] * m[14] - m[10] * m[13])) * invDet;
    result.m[1] = -(m[1] * (m[10] * m[15] - m[11] * m[14]) - m[2] * (m[9] * m[15] - m[11] * m[13]) + m[3] * (m[9] * m[14] - m[10] * m[13])) * invDet;
    result.m[2] = (m[1] * (m[6] * m[15] - m[7] * m[14]) - m[2] * (m[5] * m[15] - m[7] * m[13]) + m[3] * (m[5] * m[14] - m[6] * m[13])) * invDet;
    result.m[3] = -(m[1] * (m[6] * m[11] - m[7] * m[10]) - m[2] * (m[5] * m[11] - m[7] * m[9]) + m[3] * (m[5] * m[10] - m[6] * m[9])) * invDet;

    result.m[4] = -(m[4] * (m[10] * m[15] - m[11] * m[14]) - m[6] * (m[8] * m[15] - m[11] * m[12]) + m[7] * (m[8] * m[14] - m[10] * m[12])) * invDet;
    result.m[5] = (m[0] * (m[10] * m[15] - m[11] * m[14]) - m[2] * (m[8] * m[15] - m[11] * m[12]) + m[3] * (m[8] * m[14] - m[10] * m[12])) * invDet;
    result.m[6] = -(m[0] * (m[6] * m[15] - m[7] * m[14]) - m[2] * (m[4] * m[15] - m[7] * m[12]) + m[3] * (m[4] * m[14] - m[6] * m[12])) * invDet;
    result.m[7] = (m[0] * (m[6] * m[11] - m[7] * m[10]) - m[2] * (m[4] * m[11] - m[7] * m[8]) + m[3] * (m[4] * m[10] - m[6] * m[8])) * invDet;

    result.m[8] = (m[4] * (m[9] * m[15] - m[11] * m[13]) - m[5] * (m[8] * m[15] - m[11] * m[12]) + m[7] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[9] = -(m[0] * (m[9] * m[15] - m[11] * m[13]) - m[1] * (m[8] * m[15] - m[11] * m[12]) + m[3] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[10] = (m[0] * (m[5] * m[15] - m[7] * m[13]) - m[1] * (m[4] * m[15] - m[7] * m[12]) + m[3] * (m[4] * m[13] - m[5] * m[12])) * invDet;
    result.m[11] = -(m[0] * (m[5] * m[11] - m[7] * m[9]) - m[1] * (m[4] * m[11] - m[7] * m[8]) + m[3] * (m[4] * m[9] - m[5] * m[8])) * invDet;

    result.m[12] = -(m[4] * (m[9] * m[14] - m[10] * m[13]) - m[5] * (m[8] * m[14] - m[10] * m[12]) + m[6] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[13] = (m[0] * (m[9] * m[14] - m[10] * m[13]) - m[1] * (m[8] * m[14] - m[10] * m[12]) + m[2] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[14] = -(m[0] * (m[5] * m[14] - m[6] * m[13]) - m[1] * (m[4] * m[14] - m[6] * m[12]) + m[2] * (m[4] * m[13] - m[5] * m[12])) * invDet;
    result.m[15] = (m[0] * (m[5] * m[10] - m[6] * m[9]) - m[1] * (m[4] * m[10] - m[6] * m[8]) + m[2] * (m[4] * m[9] - m[5] * m[8])) * invDet;

    return result;
}

Matrix4 Matrix4::CreateTranslation(const Vector3& translation) {
    Matrix4 result;
    result.SetIdentity();
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

Matrix4 Matrix4::CreateRotation(const Quaternion& rotation) {
    return rotation.ToRotationMatrix();
}

Matrix4 Matrix4::CreateRotationX(float angle) {
    Matrix4 result;
    result.SetIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[5] = c;
    result.m[6] = s;
    result.m[9] = -s;
    result.m[10] = c;
    return result;
}

Matrix4 Matrix4::CreateRotationY(float angle) {
    Matrix4 result;
    result.SetIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0] = c;
    result.m[2] = -s;
    result.m[8] = s;
    result.m[10] = c;
    return result;
}

Matrix4 Matrix4::CreateRotationZ(float angle) {
    Matrix4 result;
    result.SetIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0] = c;
    result.m[1] = s;
    result.m[4] = -s;
    result.m[5] = c;
    return result;
}

Matrix4 Matrix4::CreateScale(const Vector3& scale) {
    Matrix4 result;
    result.SetIdentity();
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

Matrix4 Matrix4::CreatePerspective(float fov, float aspect, float nearPlane, float farPlane) {
    Matrix4 result;
    result.SetZero();
    
    float f = 1.0f / std::tan(fov * 0.5f);
    float rangeInv = 1.0f / (nearPlane - farPlane);
    
    result.m[0] = f / aspect;
    result.m[5] = f;
    result.m[10] = (nearPlane + farPlane) * rangeInv;
    result.m[11] = -1.0f;
    result.m[14] = nearPlane * farPlane * rangeInv * 2.0f;
    
    return result;
}

Matrix4 Matrix4::CreateOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Matrix4 result;
    result.SetZero();
    
    result.m[0] = 2.0f / (right - left);
    result.m[5] = 2.0f / (top - bottom);
    result.m[10] = -2.0f / (farPlane - nearPlane);
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.m[15] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 f = (target - eye).Normalized();
    Vector3 s = Vector3::Cross(f, up).Normalized();
    Vector3 u = Vector3::Cross(s, f);
    
    Matrix4 result;
    result.SetIdentity();
    result.m[0] = s.x;
    result.m[4] = s.y;
    result.m[8] = s.z;
    result.m[1] = u.x;
    result.m[5] = u.y;
    result.m[9] = u.z;
    result.m[2] = -f.x;
    result.m[6] = -f.y;
    result.m[10] = -f.z;
    result.m[12] = -Vector3::Dot(s, eye);
    result.m[13] = -Vector3::Dot(u, eye);
    result.m[14] = Vector3::Dot(f, eye);
    
    return result;
}

const Matrix4 Matrix4::Identity = Matrix4();
const Matrix4 Matrix4::Zero = []() { Matrix4 m; m.SetZero(); return m; }();

} // namespace SmartRenderer
