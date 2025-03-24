#include <fstream>
#include <vector>
#include "DebugSample.h"

std::vector<DebugSample> g_debugSamples;

void DebugSample::print(std::ostream& file, const bool headerRow) {
    if (headerRow) {
        file << "pos.x,pos.y,posA.x,posA.y,posB.x,posB.y,lerp,a,b,value";
    } else {
        file << pos.x << "," << pos.y << ","
                << posA.x << "," << posA.y << ","
                << posB.x << "," << posB.y << ","
                << lerp << ","
                << a << "," << b << ","
                << val;
    }
}

void DebugSample::record(const DebugSample& sample) {
    g_debugSamples.push_back(sample);
}
