#ifndef PLAYER_COMPONENT_H_
#define PLAYER_COMPONENT_H_

#include <util/ability.hpp>

struct PlayerComponent {
  int id = 0;
  AbilityID primary_ability = NULL_ABILITY;
  AbilityID secondary_ability = NULL_ABILITY;
};

#endif  // PLAYER_COMPONENT_H_