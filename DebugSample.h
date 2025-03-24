#pragma once
#include <vector>
#include <iostream>
#include "Vec2.h"

struct DebugSample {
    Vec2 pos;
    Vec2 posA;
    Vec2 posB;
    double lerp;
    double a;
    double b;
    double val;

    void print(std::ostream& file, const bool headerRow = false);

    static void record(const DebugSample& sample);
};

extern std::vector<DebugSample> g_debugSamples;


