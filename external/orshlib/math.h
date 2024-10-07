#pragma once

namespace OL {

    struct Vector2 {
        float x, y;

        Vector2 operator+(const Vector2& other) {
            return Vector2(x + other.x, y + other.y);
        }

        Vector2& operator+=(const Vector2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }
    };

    struct Vector3 {
        float x, y, z;
    };

    struct Vector4 {
        float x, y, z, w;
    };

    struct Matrix4 {
        Vector4 x, y, z, w;
    };

    static constexpr Matrix4 MAT4_IDENTITY = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
    };

    static constexpr Matrix4 orthographic_projection(f32 left, f32 right, f32 bottom, f32 top, f32 near_plane, f32 far_plane) {
        Matrix4 projection = { 0 };

        projection.x.x = 2.0f / (right - left);
        projection.y.y = 2.0f / (top - bottom);
        projection.z.z = -2.0f / (far_plane - near_plane);
        projection.x.w = -(right + left) / (right - left);
        projection.y.w = -(top + bottom) / (top - bottom);
        projection.z.w = -(far_plane + near_plane) / (far_plane - near_plane);
        projection.w.w = 1.0f;

        return projection;
    };

    static constexpr Matrix4 ORTHO = orthographic_projection(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
}