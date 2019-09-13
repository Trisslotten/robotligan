#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include <util/enums/ability_id.hpp>

struct PlayerComponent {
  int id = 0;
  AbilityID primary_ability = NULL_ABILITY;
  AbilityID secondary_ability = NULL_ABILITY;
};

#endif  // PLAYER_COMPONENT_HPP_