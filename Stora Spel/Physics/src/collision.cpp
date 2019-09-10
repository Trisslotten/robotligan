#include "collision.h"

#include <glm.hpp>

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