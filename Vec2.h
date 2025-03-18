#pragma once

struct Vec2 {
    float x;
    float y;

    Vec2();
    Vec2(float _x, float _y);
    void normalize();
    float lengthSquared() const;
    float length();
    bool equals(const Vec2 &other);
};