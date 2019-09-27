#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include "../shared/shared.hpp"

struct PlayerComponent {
  PlayerID id = -1;
  float pitch = 0.f;
  float yaw = 0.f;
};

#endif  // PLAYER_COMPONENT_HPP_