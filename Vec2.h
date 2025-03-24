#pragma once

#include <fstream>

struct Vec2 {
    double x;
    double y;

    Vec2();

    Vec2(double _x, double _y);

    void normalize();

    double lengthSquared() const;

    double length();

    bool equals(const Vec2& other);

    bool inRect(const Vec2& boundsMin, const Vec2& boundsMax);

    Vec2& operator+=(const Vec2& other);
};
