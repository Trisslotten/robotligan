#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include <glm.hpp>

struct PhysicsComponent {
  glm::vec3 velocity;
  float friction;
};

#endif  // PHYSICS_COMPONENT_H