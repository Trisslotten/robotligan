#include "ability.hpp"

AbilityID Ability::RandomAbilityID() {
  int id = rand() % (NUM_OF_ABILITY_IDS - 1) + 1;
  return (AbilityID)id;
}

void Ability::TriggerAbility(entt::registry& registry) {
  AbilityID temp_id = NULL_ABILITY;

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
      CreateMissileEntity(registry);
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
void Ability::CreateMissileEntity(entt::registry& registry) {
  // Create new entity
  auto entity = registry.create();

  // Assign it a missile_component
  registry.assign<MissileComponent>(entity/*, other parameters (i.e. start
    pos, velocity, time etc*/);
}
