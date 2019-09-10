#ifndef PHYSICS_INCLUDE_COLLISION_H_
#define PHYSICS_INCLUDE_COLLISION_H_

#include "boundingboxes.h"

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

EXPORT bool Intersect(const OBB& o1, const OBB& o2);
EXPORT bool Intersect(const Sphere& s1, const Sphere& s2);

#endif  // PHYSICS_INCLUDE_COLLISION_H_