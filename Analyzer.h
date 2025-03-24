#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "Vec2.h"

struct Sample {
  Vec2 pos;
  Vec2 posA;
  Vec2 posB;
  double lerp;
  double val;

  void print(std::ostream& file, bool headerRow = false);
};

class Analyzer {
private:
  static double sample(const std::vector<unsigned char>& buffer, const Vec2& mapDims, const Vec2& position);

public:
  static std::vector<unsigned char> LoadHeightmap(const std::string& path);

  static Vec2 getDiagonalIntersection(const Vec2& a, const Vec2& b);

  static double CalculatePathLength(const std::vector<unsigned char>& heightMap, const Vec2& mapDims, const Vec2& start, const Vec2& end);

};
