#include "collision.h"

#include <glm.hpp>

bool Intersect(const Sphere& s1, const Sphere& s2) {
  float d = glm::distance(s1.center, s2.center);

  return d <= (s1.radius + s2.radius);
}