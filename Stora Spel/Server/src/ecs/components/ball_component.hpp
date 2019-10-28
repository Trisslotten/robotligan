#ifndef BALL_COMPONENT_HPP_
#define BALL_COMPONENT_HPP_

#include "shared/shared.hpp"
#include <glm/gtx/quaternion.hpp>

struct BallComponent {
  bool is_real;
  bool is_airborne;
  glm::quat rotation;
  PlayerID last_touch;
  PlayerID prev_touch;
  bool is_homing = false;
  long homer_cid = -1;
  bool is_super_striked = false;
  unsigned int faker_team = TEAM_RED;
  bool should_be_destroyed = false;
};

#endif  // BALL_COMPONENT_HPP_
