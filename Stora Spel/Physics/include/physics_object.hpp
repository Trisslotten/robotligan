#ifndef PHYSICS_OBJECT_HPP_
#define PHYSICS_OBJECT_HPP_

#include <vec3.hpp>

namespace physics {

struct PhysicsObject {
  glm::vec3 position;
  glm::vec3 velocity;
  float friction;
  bool airborne;
};
}  // namespace physics

#endif  // PHYSICS_OBJECT_HPP_