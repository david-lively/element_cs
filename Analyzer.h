#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "Vec2.h"


class Analyzer {
private:
  static double sample(const std::vector<unsigned char>& buffer, const Vec2& mapDims, const Vec2& position);

public:
  static std::vector<unsigned char> loadHeightmap(const std::string& path);

  static Vec2 getDiagonalIntersection(const Vec2& a, const Vec2& b);

  static double calculatePathLength(const std::vector<unsigned char>& heightMap, const Vec2& mapDims, const Vec2& start,
                                    const Vec2& end);
};
