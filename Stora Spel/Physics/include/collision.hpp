#ifndef PHYSICS_INCLUDE_COLLISION_HPP_
#define PHYSICS_INCLUDE_COLLISION_HPP_

#include "boundingboxes.hpp"

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif


namespace physics {
struct IntersectData {
  glm::vec3 normal;
  glm::vec3 move_vector;
  bool collision;
};

EXPORT IntersectData Intersect(const OBB& o1, const OBB& o2);
EXPORT IntersectData Intersect(const Sphere& s1, const Sphere& s2);
EXPORT IntersectData Intersect(const Sphere& s, const OBB& o);
EXPORT IntersectData Intersect(const Arena& a, const Sphere& s);
EXPORT IntersectData Intersect(const Arena& a, const OBB& o);
EXPORT IntersectData Intersect(const MeshHitbox& m, const Sphere& s);
EXPORT IntersectData Intersect(const MeshHitbox& m, const OBB& o);
}  // namespace physics

#endif  // PHYSICS_INCLUDE_COLLISION_HPP_