#ifndef PHYSICS_INCLUDE_COLLISION_HPP_
#define PHYSICS_INCLUDE_COLLISION_HPP_

#include "boundingboxes.hpp"

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

namespace physics {
EXPORT bool Intersect(const OBB& o1, const OBB& o2);
EXPORT bool Intersect(const Sphere& s1, const Sphere& s2);
EXPORT bool Intersect(const Sphere& s, const OBB& o, glm::vec3* normal);
EXPORT bool Intersect(const Arena& a, const Sphere& s, glm::vec3* normal);
EXPORT bool Intersect(const Arena& a, const OBB& o, glm::vec3* pos);
}  // namespace physics

#endif  // PHYSICS_INCLUDE_COLLISION_HPP_