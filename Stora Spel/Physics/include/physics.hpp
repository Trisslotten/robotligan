#ifndef PHYSICS_INCLUDE_PHYSICS_HPP_
#define PHYSICS_INCLUDE_PHYSICS_HPP_

#include "physics_object.hpp"

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

namespace physics {
EXPORT void SetGravity(float _g);
EXPORT void Update(PhysicsObject* po, float dt);
}  // namespace physics

#endif  // PHYSICS_INCLUDE_PHYSICS_HPP_