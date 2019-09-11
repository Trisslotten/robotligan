#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <glm.hpp>

struct PhysicsObject {
  glm::vec3 position;
  glm::vec3 velocity;
  float friction;
};

#endif  // PHYSICS_OBJECT_H