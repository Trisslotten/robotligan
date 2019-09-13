#ifndef PHYSICS_COMPONENT_HPP
#define PHYSICS_COMPONENT_HPP

#include <glm/glm.hpp>

struct PhysicsComponent {
  glm::vec3 velocity;
  float friction;
};

#endif  // PHYSICS_COMPONENT_HPP