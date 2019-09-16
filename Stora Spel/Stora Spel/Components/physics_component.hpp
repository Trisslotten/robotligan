#ifndef PHYSICS_COMPONENT_HPP_
#define PHYSICS_COMPONENT_HPP_

#include <glm/glm.hpp>

struct PhysicsComponent {
  glm::vec3 velocity;
  bool is_airborne;
  float friction;
};

#endif  // PHYSICS_COMPONENT_HPP_