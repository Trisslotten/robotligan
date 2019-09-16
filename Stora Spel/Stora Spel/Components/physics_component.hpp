#ifndef PHYSICS_COMPONENT_HPP_
#define PHYSICS_COMPONENT_HPP_

#include <glm/glm.hpp>

struct PhysicsComponent {
  glm::vec3 velocity;
  float friction;
  bool is_airborne = false;
};

#endif  // PHYSICS_COMPONENT_HPP_