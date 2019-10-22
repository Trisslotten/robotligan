#ifndef PHYSICS_OBJECT_HPP_
#define PHYSICS_OBJECT_HPP_

#include <glm/vec3.hpp>

namespace physics {

struct PhysicsObject {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;
  float friction;
  bool airborne;
  float max_speed;
};
}  // namespace physics

#endif  // PHYSICS_OBJECT_HPP_