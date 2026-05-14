#pragma once
#include <cmath>
#include <algorithm>

namespace math {

struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    float Dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    float Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    Vec3 Normalize() const {
        float len = Length();
        if (len == 0) return Vec3(0, 0, 0);
        return Vec3(x / len, y / len, z / len);
    }
    
    Vec3 Cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};

struct Vec2 {
    float x, y;
    
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
    
    Vec3 Transform(const Vec3& v) const {
        float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
        float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
        float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
        float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];
        
        if (w != 0) {
            x /= w;
            y /= w;
            z /= w;
        }
        return Vec3(x, y, z);
    }
};


struct BoundingBox3D {
    Vec3 center;
    float width;
    float height;
    float depth;
    
    BoundingBox3D() : center(0,0,0), width(0), height(0), depth(0) {}
    BoundingBox3D(Vec3 c, float w, float h, float d) : center(c), width(w), height(h), depth(d) {}

    void GetCorners(Vec3 corners[8]) const {
        float halfW = width * 0.5f;
        float halfH = height * 0.5f;
        float halfD = depth * 0.5f;

        corners[0] = Vec3(center.x - halfW, center.y - halfH, center.z - halfD); // Front-Left
        corners[1] = Vec3(center.x + halfW, center.y - halfH, center.z - halfD); // Front-Right
        corners[2] = Vec3(center.x + halfW, center.y - halfH, center.z + halfD); // Back-Right
        corners[3] = Vec3(center.x - halfW, center.y - halfH, center.z + halfD); // Back-Left
        corners[4] = Vec3(center.x - halfW, center.y + halfH, center.z - halfD); // Front-Left
        corners[5] = Vec3(center.x + halfW, center.y + halfH, center.z - halfD); // Front-Right
        corners[6] = Vec3(center.x + halfW, center.y + halfH, center.z + halfD); // Back-Right
        corners[7] = Vec3(center.x - halfW, center.y + halfH, center.z + halfD); // Back-Left
    }
};


inline Vec2 WorldToScreen(const Vec3& worldPos, const Matrix4x4& viewProj, float screenW, float screenH) {
    Vec3 clip = viewProj.Transform(worldPos);
    float screenX = (clip.x + 1.0f) * 0.5f * screenW;
    float screenY = (1.0f - clip.y) * 0.5f * screenH;
    
    return Vec2(screenX, screenY);
}

inline float Distance3D(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
    return Vec3(
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t)
    );
}

} // namespace math
