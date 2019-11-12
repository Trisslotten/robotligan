#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include "shared/shared.hpp"
#include "util/timer.hpp"

struct PlayerComponent {  // Client side
  // PlayerID id = -1;
  Timer step_timer;

  // Animation info (Set in AnimationSystem, but might be usefull outside of
  // animations)
  float sprint_coeff = 0.f;
  bool sprinting = false;
  bool running = false;
  bool jumping = false;
  float jump_force = 0.f;

  glm::vec3 vel_dir = glm::vec3(1.f, 0.f, 0.f);
  glm::vec3 look_dir = glm::vec3(1.f, 0.f, 0.f);
  glm::vec3 move_dir = glm::vec3(1.f, 0.f, 0.f);
  bool can_jump = false;
};

#endif  // PLAYER_COMPONENT_HPP_