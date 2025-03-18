#include <limits>
#include <cassert>
#include <cmath>
#include "Vec2.h"

using namespace std;
Vec2::Vec2() : x(0), y(0) {
}

Vec2::Vec2(float _x, float _y) : x(_x), y(_y) {
}

void Vec2::normalize() {
    float magnitude = sqrt(x * x + y * y);
    if (magnitude > 0) {
        x /= magnitude;
        y /= magnitude;
    }
}

float Vec2::lengthSquared() const {
    return x * x + y * y;
}

float Vec2::length() {
    return sqrt(lengthSquared());
}

bool Vec2::equals(const Vec2 &other) {
    return abs(x - other.x) < numeric_limits<float>::epsilon() &&
           abs(y - other.y) < numeric_limits<float>::epsilon();
}
