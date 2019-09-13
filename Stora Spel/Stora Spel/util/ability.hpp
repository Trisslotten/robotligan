#ifndef ABILITY_HPP_
#define ABILITY_HPP_

#include <stdlib.h>
#include <entt.hpp>

#include "enums/ability_id.hpp"
#include "Ability Components/missile_component.hpp"
#include "player_component.hpp"
/*
 NTS: Include other ability components here
*/	

class Ability {
 public:
  Ability() = delete;
  static AbilityID RandomAbilityID();
  static void TriggerAbility(entt::registry &registry,
                             PlayerComponent &player_component,
                             bool secondary_ability);
  static void CreateMissileEntity(entt::registry &registry,
                                  PlayerComponent &player_component);
};

#endif  // !ABILITY_HPP_
