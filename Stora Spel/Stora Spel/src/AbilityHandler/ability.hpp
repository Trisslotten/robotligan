#ifndef ABILITY_HPP_
#define ABILITY_HPP_

#include <stdlib.h>
#include <entt.hpp>

#include "Ability Components/missile_component.hpp"
// NTS: Include other ability components here

enum AbilityID {
  NULL_ABILITY,
  BUILD_WALL,
  FAKE_BALL,
  FORCE_PUSH,
  GRAVITY_CHANGE,
  HOMING_BALL,
  INVISIBILITY,
  MISSILE,
  SUPER_STRIKE,
  SWITCH_GOALS,
  TELEPORT,
  // Fill with more ability and passive boosts
  NUM_OF_ABILITY_IDS
};

class Ability {
 public:
  Ability() = delete;
  AbilityID RandomAbilityID();
  void TriggerAbility(entt::registry &registry /*Player*/);
  void CreateMissileEntity(entt::registry &registry);
};

#endif  // !ABILITY_HPP_
