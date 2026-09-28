#pragma once

template<typename T>
struct Vector2D {
    T r[2]{};

    Vector2D() = default;
    Vector2D(T x, T y) : r{x, y} {}

    T& operator[](int i) {
        return r[i];
    }

    const T& operator[](int i) const {
        return r[i];
    }

    bool operator==(const Vector2D&) const = default;
};