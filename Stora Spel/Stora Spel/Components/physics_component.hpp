#ifndef PHYSICS_COMPONENT_HPP_
#define PHYSICS_COMPONENT_HPP_

#include <glm.hpp>

struct PhysicsComponent {
  glm::vec3 velocity;
  float friction;
};

#endif  // PHYSICS_COMPONENT_HPP_