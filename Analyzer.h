#pragma once

#include <string>
#include <vector>
#include "Vec2.h"

class Analyzer {
  private:
    std::vector<unsigned char> m_beforeData;
    std::vector<unsigned char> m_afterData;
    Vec2 m_mapDims;
    float sample(const std::vector<unsigned char>& buffer, const Vec2& position);

  public:
    std::vector<unsigned char>& data() { return m_beforeData;}
    static std::vector<unsigned char> LoadHeightmap(const std::string& path);
    void Load(const Vec2& dims, const std::string& before, const std::string& after);
    static Vec2 getDiagonalIntersection(const Vec2 &a, const Vec2 &b);
    float CalculatePathLength(const std::vector<unsigned char>& heightMap, const Vec2& start, const Vec2& end);
    int CalculatePathLengthDelta(const Vec2& stat, const Vec2& end);
};
