#include "collision.h"

#include <glm.hpp>
#include <vector>

struct Corners {
  glm::vec3 corners[8];
};

Corners getCorners(const OBB& obb) {
  glm::vec3 normals[3] = {};
  normals[0] = obb.normals[0] * obb.extents[0];
  normals[1] = obb.normals[1] * obb.extents[1];
  normals[2] = obb.normals[2] * obb.extents[2];
  Corners c = {};
  c.corners[0] = obb.center + (normals[0] + normals[1] + normals[2]);
  c.corners[1] = obb.center + (-normals[0] + normals[1] + normals[2]);
  c.corners[2] = obb.center + (normals[0] - normals[1] + normals[2]);
  c.corners[3] = obb.center + (normals[0] + normals[1] - normals[2]);
  c.corners[4] = obb.center + (-normals[0] - normals[1] + normals[2]);
  c.corners[5] = obb.center + (-normals[0] + normals[1] - normals[2]);
  c.corners[6] = obb.center + (normals[0] - normals[1] - normals[2]);
  c.corners[7] = obb.center + (-normals[0] - normals[1] - normals[2]);

  return c;
}

void SATtest(const glm::vec3& axis, const Corners& c, float* min, float* max) {
  *min = 10000000.f;   // infinity
  *max = -10000000.f;  // -infinity

  for (int i = 0; i < 8; ++i) {
    float dot_val = glm::dot(axis, c.corners[i]);
    if (dot_val < *min) *min = dot_val;
    if (dot_val > *max) *max = dot_val;
  }
}

bool isBetween(float val, float lower, float upper) {
  return lower <= val && val <= upper;
}

bool overlaps(float min1, float max1, float min2, float max2) {
  return isBetween(min2, min1, max1) || isBetween(min1, min2, max2);
}

bool Intersect(const Sphere& s1, const Sphere& s2) {
  float d = glm::distance(s1.center, s2.center);

  return d <= (s1.radius + s2.radius);
}

bool Intersect(const Sphere& s, const OBB& o) {
  glm::vec3 retPt = o.center;
  glm::vec3 d = s.center - o.center;

  for (int i = 0; i < 3; ++i) {
    float dist = glm::dot(d, o.normals[i]);
    if (dist > o.extents[i]) dist = o.extents[i];
    if (dist < -o.extents[i]) dist = -o.extents[i];
    retPt += dist * o.normals[i];
  }
  glm::vec3 v = retPt - s.center;
  return glm::dot(v, v) <= s.radius * s.radius;
}

bool Intersect(const OBB& o1, const OBB& o2) {
  glm::vec3 normals[3] = {};
  normals[0] = o1.normals[0] * o1.extents[0];
  normals[1] = o1.normals[1] * o1.extents[1];
  normals[2] = o1.normals[2] * o1.extents[2];
  Corners c1 = getCorners(o1);
  Corners c2 = getCorners(o2);

  std::vector<glm::vec3> testnormals;
  testnormals.reserve(15);

  for (int i = 0; i < 3; ++i) {
    // glm::vec3 n1 = o1.normals[i] * o1.extents[i];
    testnormals.push_back(o1.normals[i]);
    for (int j = 0; j < 3; ++j) {
      if (i == 0) testnormals.push_back(o2.normals[j]);
      testnormals.push_back(glm::cross(o1.normals[i], o2.normals[j]));
    }
  }

  for (int i = 0; i < testnormals.size(); ++i) {
    float min1, max1, min2, max2;
    SATtest(testnormals[i], c1, &min1, &max1);
    SATtest(testnormals[i], c2, &min2, &max2);

    if (!overlaps(min1, max1, min2, max2)) {
      return false;
    }
  }

  return true;
}