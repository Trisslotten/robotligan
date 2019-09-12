#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <vec3.hpp>

namespace physics {

struct PhysicsObject {
  glm::vec3 position;
  glm::vec3 velocity;
  float friction;
  bool airborne;
};
}  // namespace physics

#endif  // PHYSICS_OBJECT_H