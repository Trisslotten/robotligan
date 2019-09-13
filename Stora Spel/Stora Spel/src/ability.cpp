//#include "util/ability.hpp"
#include <util/ability.hpp>

AbilityID Ability::RandomAbilityID() {
  int id = rand() % (NUM_OF_ABILITY_IDS - 1) + 1;
  return (AbilityID)id;
}

void Ability::TriggerAbility(entt::registry& registry,
                             PlayerComponent& player_component,
                             bool secondary_ability) {
  // NTS: Get this value from player later
  AbilityID temp_id = NULL_ABILITY;

  if (!secondary_ability) {
    temp_id = player_component.primary_ability;
  } else {
    temp_id = player_component.secondary_ability;
  }

  switch (temp_id) {
    case NULL_ABILITY:
      break;
    case BUILD_WALL:
      break;
    case FAKE_BALL:
      break;
    case FORCE_PUSH:
      break;
    case GRAVITY_CHANGE:
      break;
    case HOMING_BALL:
      break;
    case INVISIBILITY:
      break;
    case MISSILE:
      CreateMissileEntity(registry, player_component);
      break;
    case SUPER_STRIKE:
      break;
    case SWITCH_GOALS:
      break;
    case TELEPORT:
      break;
    default:
      break;
  }
}

// NTS: Example function
void Ability::CreateMissileEntity(entt::registry& registry,
                                  PlayerComponent& player_component) {
  // Create new entity
  auto entity = registry.create();

  // Assign it a missile_component
  registry.assign<MissileComponent>(entity/*, other parameters (i.e. start
    pos, velocity, time etc*/);
}
