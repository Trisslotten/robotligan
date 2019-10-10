#ifndef PHYSICS_COMPONENT_HPP_
#define PHYSICS_COMPONENT_HPP_

#include <glm/glm.hpp>

#include "util/global_settings.hpp"

struct PhysicsComponent {
  glm::vec3 velocity;
  bool is_airborne = false;
  float friction = GlobalSettings::Access()->ValueOf("PHYSICS_FRICTION");
};

#endif  // PHYSICS_COMPONENT_HPP_