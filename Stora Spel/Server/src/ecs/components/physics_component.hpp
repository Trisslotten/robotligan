#ifndef PHYSICS_COMPONENT_HPP_
#define PHYSICS_COMPONENT_HPP_

#include <glm/glm.hpp>

#include "..//util/global_settings.hpp"

struct PhysicsComponent {
  glm::vec3 velocity = glm::vec3(0.f);
  glm::vec3 acceleration = glm::vec3(0.f);
  bool is_airborne = false;
  float friction = GlobalSettings::Access()->ValueOf("PHYSICS_FRICTION");
  float max_speed = 1000.f;
};

#endif  // PHYSICS_COMPONENT_HPP_