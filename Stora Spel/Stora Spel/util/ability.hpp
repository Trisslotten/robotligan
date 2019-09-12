#ifndef ABILITY_HPP_
#define ABILITY_HPP_

#include <stdlib.h>
#include <entt.hpp>

#include "Ability Components/missile_component.hpp"
// NTS: Include other ability components here

namespace ability {

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

AbilityID RandomAbilityID() {
  int id = rand() % (NUM_OF_ABILITY_IDS - 1) + 1;
  return (AbilityID)id;
}


//Declare functions used my TriggerAbility()
void CreateMissileEntity(entt::registry &registry);

void TriggerAbility(entt::registry &registry /*Player*/) {
  AbilityID temp_id = NULL_ABILITY;

  switch (temp_id) {
    case ability::NULL_ABILITY:
      break;
    case ability::BUILD_WALL:
      break;
    case ability::FAKE_BALL:
      break;
    case ability::FORCE_PUSH:
      break;
    case ability::GRAVITY_CHANGE:
      break;
    case ability::HOMING_BALL:
      break;
    case ability::INVISIBILITY:
      break;
    case ability::MISSILE:
      ability::CreateMissileEntity(registry);
      break;
    case ability::SUPER_STRIKE:
      break;
    case ability::SWITCH_GOALS:
      break;
    case ability::TELEPORT:
      break;
    default:
      break;
  }
}

// NTS: Example function
void CreateMissileEntity(entt::registry &registry) {
  // Create new entity
  auto entity = registry.create();

  // Assign it a missile_component
  registry.assign<MissileComponent>(
      entity /*, other parameters (i.e. start pos, velocity, time etc*/);
}

};  // namespace ability

#endif  // !ABILITY_HPP_
