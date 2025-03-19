//
// Created by David Lively on 3/17/25.
//

#include <fstream>
#include <iostream>
#include <assert.h>
#include <iterator>

#include <vector>
#include "Analyzer.h"

#define FLOAT_EPSILON numeric_limits<float>::epsilon()
#define FLOAT_MAX numeric_limits<float>::max()

using namespace std;

vector<unsigned char> Analyzer::LoadHeightmap(const std::string& path) {
  ifstream input(path, ios::binary);
  vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
  return buffer;
}

void Analyzer::Load(const Vec2& dims, const std::string& before, const std::string& after) {
  m_beforeData = LoadHeightmap(before);
  m_afterData = LoadHeightmap(after);
  assert(m_beforeData.size() == m_afterData.size() && "Buffer sizes do not match");
  assert(dims.x * dims.y == m_beforeData.size() && "Specified dimensions do not buffer");
  m_mapDims = dims;
}

Vec2 Analyzer::getDiagonalIntersection(const Vec2& a, const Vec2& b) {
  float left = (int) ((a.x + b.x) / 2.0f);
  float top = (int) ((a.y + b.y) / 2.0f);
  float cx = left + 1;
  float cy = top;
  float dx = left;
  float dy = top + 1;

  Vec2 result(FLOAT_MAX, FLOAT_MAX);

  float a1 = b.y - a.y;
  float b1 = a.x - b.x;
  float c1 = a1 * a.x + b1 * a.y;

  float a2 = dy - cy;
  float b2 = cx - dx;
  float c2 = a2 * cx + b2 * cy;

  float det = a1 * b2 - a2 * b1;

  if (abs(det) > FLOAT_EPSILON) {
    result.x = (b2 * c1 - b1 * c2) / det;
    result.y = (a1 * c2 - a2 * c1) / det;

    if (result.x < left || result.x >= left + 1 || result.y < top || result.y >= top + 1) {
      result.x = FLOAT_MAX;
      result.y = FLOAT_MAX;
    }
  }

  return result;
}

float frac(float v) {
  return v - (int) v;
}

#define OFFSET_OF(p) ((unsigned int)p.y * m_mapDims.x + (unsigned int)p.x)
#define SAMPLE(p) buffer[OFFSET_OF(p)]

float Analyzer::sample(const vector<unsigned char>& buffer, const Vec2& position) {
  assert(position.x >= 0 && position.x < m_mapDims.x
    && position.y >= 0 && position.y < m_mapDims.y
    && "Position out of bounds");

  float dv = frac(position.x);
  bool onVertical = dv <= FLOAT_EPSILON || (1 - dv) <= FLOAT_EPSILON;

  float dh = frac(position.y);
  bool onHorizontal = dh <= FLOAT_EPSILON || (1 - dh) <= FLOAT_EPSILON;

  if (onVertical && onHorizontal) {
    // centered on a pixel
    return SAMPLE(position);
  }

  const unsigned int y0 = (unsigned int) floor(position.y);
  const unsigned int y1 = (unsigned int) ceil(position.y);
  const unsigned int x0 = (unsigned int) floor(position.x);
  const unsigned int x1 = (unsigned int) ceil(position.x);

  float a = 0;
  float b = 0;
  float lerpValue = 0.0;

  if (onVertical) {
    // interpolate in Y
    a = buffer[y0 * m_mapDims.x + x0];
    b = buffer[y1 * m_mapDims.x + x0];
    lerpValue = frac(position.y);
  } else if (onHorizontal) {
    // interpolate in X
    a = buffer[y0 * m_mapDims.x + x0];
    b = buffer[y0 * m_mapDims.x + x1];
    lerpValue = frac(position.x);
  } else {
    // diagonal
    a = buffer[y0 * m_mapDims.x + x1];
    b = buffer[y1 * m_mapDims.x + x0];
    float fx = frac(position.x);
    float fy = frac(position.y);
    lerpValue = sqrt(fx * fx + fy * fy) / sqrt(2);
  }
  assert(lerpValue >= 0 && lerpValue <= 1 && "Invalid lerp parameter.");
  return a + (b - a) * lerpValue;
}

float getSpatialDistance(const Vec2& p0, const float h0, const Vec2& p1, const float h1) {
  float dx = 30 * (p1.x - p0.x);
  float dy = 30 * (p1.y - p0.y);
  float dh = 11 * (h1 - h0);
  return sqrt(dx * dx + dy * dy + dh * dh);
}

float Analyzer::CalculatePathLength(const std::vector<unsigned char>& heightMap, const Vec2& start, const Vec2& end) {
  Vec2 rayDir(end.x - start.x, end.y - start.y);
  float maxDistance = rayDir.length();
  rayDir.normalize();

  Vec2 position = start;
  float pathLength = 0;
  float prevHeight = sample(heightMap, position);

  Vec2 stepSize;
  stepSize.x = sqrt(1 + rayDir.y * rayDir.y / (rayDir.x * rayDir.x));
  stepSize.y = sqrt(1 + rayDir.x * rayDir.x / (rayDir.y * rayDir.y));

  Vec2 rayLength;
  Vec2 stepDir;
  float distance = 0.0f;

  if (rayDir.x < 0) {
    stepDir.x = -1;
    rayLength.x = 0;
  } else {
    stepDir.x = 1;
    rayLength.x = stepSize.x;
  }

  if (rayDir.y < 0) {
    stepDir.y = -1;
    rayLength.y = 0;
  } else {
    stepDir.y = 1;
    rayLength.y = stepSize.y;
  }

  Vec2 intersection;
  Vec2 prev = position;
  int numSamples = 0;

  while (distance <= maxDistance) {
    if (rayLength.x < rayLength.y) {
      position.x += stepDir.x;
      distance = rayLength.x;
      rayLength.x += stepSize.x;
    } else {
      position.y += stepDir.y;
      distance = rayLength.y;
      rayLength.y += stepSize.y;
    }

    if (distance <= maxDistance) {
      ++numSamples;
      // next intersection (on next integer coordinate transition)
      // doesn't check diagonals
      intersection.x = start.x + rayDir.x * distance;
      intersection.y = start.y + rayDir.y * distance;

      if (!intersection.equals(prev)) {
        // check for diagonal intersection
        Vec2 di = getDiagonalIntersection(intersection, prev);
        if (di.x != FLOAT_MAX && di.y != FLOAT_MAX) {
          float height = sample(heightMap, di);
          float dh = getSpatialDistance(di, height, prev, prevHeight);
          pathLength += dh;
          prevHeight = height;
          prev = di;
        } {
          float height = sample(heightMap, intersection);
          float dh = getSpatialDistance(intersection, height, prev, prevHeight);
          pathLength += dh;
          prevHeight = height;
        }
      }
      prev = intersection;;
    }
  }
  cout << "Sampled " << numSamples << " points." << endl;

  return pathLength;
}

/*
 * This method performs a 2D raycast through the source data.
 * At each cell boundary or diagonal intersection, a height value is read
 * from each of the source heightmaps.
 * The sample point coordinates and height are then scaled to real-world dimensions
 * (30 meters in X and Y, 11 meters in height (Z)
 * We then calculate the spatial distance between the current sample point and the previous
 * sample, and add that distance to the running totals in pathLengths.
 */
Vec2 Analyzer::CalculatePathLengths(const Vec2& start, const Vec2& end) {
  Vec2 rayDir(end.x - start.x, end.y - start.y);
  float maxDistance = rayDir.length();
  rayDir.normalize();

  Vec2 position = start;

  // x is PRE data, y is POST data
  Vec2 pathLengths;
  Vec2 prevHeights(sample(m_beforeData, position), sample(m_afterData, position));

  Vec2 stepSize;
  stepSize.x = sqrt(1 + rayDir.y * rayDir.y / (rayDir.x * rayDir.x));
  stepSize.y = sqrt(1 + rayDir.x * rayDir.x / (rayDir.y * rayDir.y));

  Vec2 rayLength;
  Vec2 stepDir;
  float distance = 0.0f;

  if (rayDir.x < 0) {
    stepDir.x = -1;
    rayLength.x = 0;
  } else {
    stepDir.x = 1;
    rayLength.x = stepSize.x;
  }

  if (rayDir.y < 0) {
    stepDir.y = -1;
    rayLength.y = 0;
  } else {
    stepDir.y = 1;
    rayLength.y = stepSize.y;
  }

  Vec2 intersection;
  Vec2 prev = position;

  int numSamples = 0;

  while (distance <= maxDistance) {
    if (rayLength.x < rayLength.y) {
      position.x += stepDir.x;
      distance = rayLength.x;
      rayLength.x += stepSize.x;
    } else {
      position.y += stepDir.y;
      distance = rayLength.y;
      rayLength.y += stepSize.y;
    }

    if (distance <= maxDistance) {
      ++numSamples;
      // next intersection (on next integer coordinate transition)
      // doesn't check diagonals
      intersection.x = start.x + rayDir.x * distance;
      intersection.y = start.y + rayDir.y * distance;

      if (!intersection.equals(prev)) {
        // check for diagonal intersection
        Vec2 di = getDiagonalIntersection(intersection, prev);
        if (di.x != FLOAT_MAX && di.y != FLOAT_MAX) {
          Vec2 heights(sample(m_beforeData, di), sample(m_afterData, di));
          pathLengths.x += getSpatialDistance(di, heights.x, prev, prevHeights.x);
          pathLengths.y += getSpatialDistance(di, heights.y, prev, prevHeights.y);
          prevHeights = heights;
          prev = di;
        }
        {
          Vec2 heights(sample(m_beforeData, intersection), sample(m_afterData, intersection));
          pathLengths.x += getSpatialDistance(intersection, heights.x, prev, prevHeights.x);
          pathLengths.y += getSpatialDistance(intersection, heights.y, prev, prevHeights.y);
          prevHeights = heights;
        }
      }
      prev = intersection;;
    }
  }
  cout << "Sampled " << numSamples << " points." << endl;
  return pathLengths;
}

