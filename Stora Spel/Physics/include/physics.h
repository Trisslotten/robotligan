#ifndef PHYSICS_INCLUDE_PHYSICS_H_
#define PHYSICS_INCLUDE_PHYSICS_H_

#include "physics_object.h"

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

namespace physics {
EXPORT void update(PhysicsObject* po, float dt);
} // namespace physics

#endif  // PHYSICS_INCLUDE_PHYSICS_H_