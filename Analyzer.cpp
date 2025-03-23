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
constexpr float SQRT_2 = 1.414213562f;


using namespace std;

vector<unsigned char> Analyzer::LoadHeightmap(const std::string& path) {
  ifstream input(path, ios::binary);
  if (input.fail()) {
    cerr << "Could not open file `" << path << "`" << endl;
    exit(EXIT_FAILURE);
  }
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

/*
 *Calculates the intersection between
 *I'm sure there is a better way to do this.
 */
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

template<typename T>
T frac(T v) {
  return v - (int) v;
}

template<typename T>
T clamp(T v) {
  return v < 0 ? 0 : v > 1 ? 1 : v;
}

template<typename T>
T sign(T v) {
  return v < 0 ? -1 : 1;
}

float distanceSquared(const Vec2& a, const Vec2& b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return dx * dx + dy * dy;
}

float distance(const Vec2& a, const Vec2& b) {
  return sqrt(distanceSquared(a, b));
}

#define OFFSET_OF(p) ((unsigned int)p.y * m_mapDims.x + (unsigned int)p.x)
#define SAMPLE(p) buffer[OFFSET_OF(p)]

/*
 * Read an interpolated sample from the given buffer.
 * Sample positions lying on an integer X will be interpolated between their
 * left and right neighbors.
 * Those on an integer Y will be interpolated from their upper and lower neighbors.
 * If the point is exactly on a pixel (within epsilon), no interpolation is performed.
 * For sample positions within a cell, we assume it is on a diagonal edge. In that case,
 * we sample the upper-right and lower-left adjacent values and lerp between them
 * based on the sample's position on the diagonal.
 */
float Analyzer::sample(const vector<unsigned char>& buffer, const Vec2& pos) {
  int left = floor(pos.x);
  int right = ceil(pos.x);
  int top = floor(pos.y);
  int bottom = ceil(pos.y);

  if (abs(left - pos.x) <= FLOAT_EPSILON)
    right = left;
  else if (abs(right - pos.x) <= FLOAT_EPSILON)
    left = right;
  if (abs(top - pos.y) <= FLOAT_EPSILON)
    bottom = top;
  else if (abs(bottom-pos.y) <= FLOAT_EPSILON)
    top = bottom;

  Vec2 p0(right,top);
  Vec2 p1(left,bottom);
  float dist = distance(p0, p1);
  float da = distance(pos,p0);
  float lerpFactor = dist / da;

  float a = SAMPLE(p0);
  float b = SAMPLE(p1);

  return lerp(a,b,lerpFactor);
}

/*
 * Get the "real world" distance between two points (x,y, height)
 */
float getSpatialDistance(const Vec2& p0, const float h0, const Vec2& p1, const float h1) {
  /// scale to real world units.
  /// 30 meters in x and y per unit,
  float dx = 30 * (p1.x - p0.x);
  float dy = 30 * (p1.y - p0.y);
  /// 11 meters in height per unit.
  float dh = 11 * (h1 - h0);
  return sqrt(dx * dx + dy * dy + dh * dh);
}

/*
 * This method performs a 2D raycast through the source data.
 * At each cell boundary or diagonal intersection, a height value is read
 * from the heightmap.
 * The sample point coordinates and height are then scaled to real-world dimensions
 * (30 meters in X and Y, 11 meters in height (Z)
 * We then calculate the spatial distance between the current sample point and the previous
 * sample, and add that distance to the running total in pathLength.
 *
 * While it's more efficient to do both the old and new heightmaps at once since all of the
 * sample positions are identical, in practice, this doesn't provide a noticeable perf benefit
 * vs running the single-path version once for each map.
 */

float Analyzer::CalculatePathLength(const std::vector<unsigned char>& heightMap, const Vec2& start, const Vec2& end) {
  Vec2 boundsMin(min(start.x,end.x),min(start.y,end.y));
  Vec2 boundsMax (max(start.x,end.x),max(start.y,end.y));

  Vec2 current = start;
  Vec2 rayDir(end.x - start.x, end.y - start.y);
  rayDir.normalize();

  Vec2 xStep(sign(rayDir.x), abs(rayDir.y / rayDir.x) * sign(rayDir.y));
  Vec2 nextX(current.x + xStep.x, current.y + xStep.y);

  Vec2 yStep(abs(rayDir.x / rayDir.y) * sign(rayDir.x), sign(rayDir.y));
  Vec2 nextY(current.x + yStep.x, current.y + yStep.y);

  Vec2 next = current;
  float prevHeight = heightMap[start.y * m_mapDims.x + (unsigned int)start.x];
  float pathLength = 0;

  do {
    float dx = distanceSquared(current,nextX);
    float dy = distanceSquared(current,nextY);

    if (isnan(dx) && isnan(dy))
      break;

    if (dx < dy)
    {
      next = nextX;
      nextX.add(xStep);
    } else if (dx > dy)
    {
      next = nextY;
      nextY.add(yStep);
    } else
    {
      next = nextX;
      nextX.add(xStep);
      nextY.add(yStep);
    }

    // diagonal check.
    Vec2 nextD = getDiagonalIntersection(current,next);
    if (nextD.inRect(boundsMin, boundsMax))
    {
      float dh = sample(heightMap,nextD);
      float dist = getSpatialDistance(current,prevHeight, nextD, dh);
      pathLength += dist;
      prevHeight = dh;
      current = nextD;
    }

    if (next.inRect(boundsMin, boundsMax)) {
      float nextHeight = sample(heightMap, next);
      float dist = getSpatialDistance(current,prevHeight,next,nextHeight);
      pathLength += dist;
      prevHeight = nextHeight;
    }
    current = next;
  } while (current.inRect(boundsMin,boundsMax));


  return pathLength;
}


