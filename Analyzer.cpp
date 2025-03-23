#include <fstream>
#include <iostream>
#include <assert.h>
#include <vector>
#include "Analyzer.h"

#define DOUBLE_EPSION numeric_limits<double>::epsilon()
#define DOUBLE_INF numeric_limits<double>::infinity()

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

/*
 *Calculates the intersection between a ray and the "/" diagonal in a unit cell.
 *I'm sure there is a better way to do this.
 */
Vec2 Analyzer::getDiagonalIntersection(const Vec2& a, const Vec2& b) {
  double left = (int) ((a.x + b.x) * 0.5);
  double top = (int) ((a.y + b.y) * 0.5);
  double cx = left + 1;
  double cy = top;
  double dx = left;
  double dy = top + 1;

  Vec2 result(DOUBLE_INF, DOUBLE_INF);

  double a1 = b.y - a.y;
  double b1 = a.x - b.x;
  double c1 = a1 * a.x + b1 * a.y;

  double a2 = dy - cy;
  double b2 = cx - dx;
  double c2 = a2 * cx + b2 * cy;

  double oneOverDet = 1.0 / (a1 * b2 - a2 * b1);

  if (abs(oneOverDet) > DOUBLE_EPSION) {
    result.x = (b2 * c1 - b1 * c2) * oneOverDet;
    result.y = (a1 * c2 - a2 * c1) * oneOverDet;

    // if the intersection is outside of the quad...
    if (result.x < left || result.x >= left + 1 || result.y < top || result.y >= top + 1) {
      result.x = DOUBLE_INF;
      result.y = DOUBLE_INF;
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

double distanceSquared(const Vec2& a, const Vec2& b) {
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return dx * dx + dy * dy;
}

double distance(const Vec2& a, const Vec2& b) {
  return sqrt(distanceSquared(a, b));
}

#define OFFSET_OF(p) ((unsigned int)p.y * mapDims.x + (unsigned int)p.x)
#define SAMPLE(p) buffer[OFFSET_OF(p)]

/*
 * Read an interpolated sample from the given buffer.
 * Sample positions lying on an integer X will be interpolated between their neighbors on
 * the edge where the intersection lies.
 * Those on an integer Y will be interpolated from their upper and lower neighbors.
 * If the point is exactly on a pixel (within epsilon), no interpolation is performed.
 * For sample positions within a cell, we assume it is on a diagonal edge. In that case,
 * we sample the upper-right and lower-left adjacent values and lerp between them
 * based on the sample's position on the diagonal.
 */
double Analyzer::sample(const vector<unsigned char>& buffer, const Vec2& mapDims, const Vec2& pos) {
  double left = floor(pos.x);
  double right = ceil(pos.x);
  double top = floor(pos.y);
  double bottom = ceil(pos.y);

  // these `if`s could be consolidated and cleaned up quite a bit.

  if (abs(left - pos.x) <= DOUBLE_EPSION)
    right = left;
  else if (abs(right - pos.x) <= DOUBLE_EPSION)
    left = right;
  if (abs(top - pos.y) <= DOUBLE_EPSION)
    bottom = top;
  else if (abs(bottom - pos.y) <= DOUBLE_EPSION)
    top = bottom;

  double lerpFactor = 0;

  if (left != right)
    lerpFactor = (pos.x - left) / (right - left);
  else if (top != bottom)
    lerpFactor = (pos.y - top) / (bottom - top);
  else
    lerpFactor = 0;

  double a = SAMPLE(Vec2(right,top));
  double b = SAMPLE(Vec2(left,bottom));

  return lerp(a, b, lerpFactor);
}

/*
 * Get the "real world" distance between two points (x,y, height)
 */
double getSpatialDistance(const Vec2& p0, const double h0, const Vec2& p1, const double h1) {
  /// scale to real world units.
  /// 30 meters in x and y per unit,
  double dx = 30 * (p1.x - p0.x);
  double dy = 30 * (p1.y - p0.y);
  /// 11 meters in height per unit.
  double dh = 11 * (h1 - h0);
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
 * vs running the single-path version once for each map. That may not be the case for extremely large data sets.
 */

double Analyzer::CalculatePathLength(const std::vector<unsigned char>& heightMap, const Vec2& mapDims,
                                     const Vec2& start, const Vec2& end) {
  const Vec2 boundsMin(min(start.x, end.x), min(start.y, end.y));
  const Vec2 boundsMax(max(start.x, end.x), max(start.y, end.y));

  Vec2 rayDir(end.x - start.x, end.y - start.y);
  rayDir.normalize();

  const Vec2 xStep(sign(rayDir.x), abs(rayDir.y / rayDir.x) * sign(rayDir.y));
  Vec2 nextX(start.x + xStep.x, start.y + xStep.y);

  const Vec2 yStep(abs(rayDir.x / rayDir.y) * sign(rayDir.x), sign(rayDir.y));
  Vec2 nextY(start.x + yStep.x, start.y + yStep.y);

  Vec2 current = start;
  Vec2 next = current;
  double prevHeight = heightMap[start.y * mapDims.x + (unsigned int) start.x];
  double pathLength = 0;

  do {
    double dx = distanceSquared(current, nextX);
    double dy = distanceSquared(current, nextY);

    if (dx < dy) {
      next = nextX;
      nextX += xStep;
    } else if (dx > dy) {
      next = nextY;
      nextY += yStep;
    } else {
      next = nextX;
      nextX += xStep;
      nextY += yStep;
    }

    // diagonal check.
    Vec2 nextDiagonal = getDiagonalIntersection(current, next);
    if (nextDiagonal.inRect(boundsMin, boundsMax)) {
      double dh = sample(heightMap, mapDims, nextDiagonal);
      double dist = getSpatialDistance(current, prevHeight, nextDiagonal, dh);
      pathLength += dist;
      prevHeight = dh;
      current = nextDiagonal;
    }

    if (next.inRect(boundsMin, boundsMax)) {
      double nextHeight = sample(heightMap, mapDims, next);
      double dist = getSpatialDistance(current, prevHeight, next, nextHeight);
      pathLength += dist;
      prevHeight = nextHeight;
    }
    current = next;
  } while (current.inRect(boundsMin, boundsMax));

  return pathLength;
}


