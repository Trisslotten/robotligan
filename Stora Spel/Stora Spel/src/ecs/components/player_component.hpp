#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include "shared/shared.hpp"
#include "util/timer.hpp"

struct PlayerComponent {
  //PlayerID id = -1;
  Timer step_timer;
};

#endif  // PLAYER_COMPONENT_HPP_