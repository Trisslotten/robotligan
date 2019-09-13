#ifndef ABILITY_CONTROLLER_SYSTEM_HPP_
#define ABILITY_CONTROLLER_SYSTEM_HPP_

#include <position.h>
#include <ability_component.hpp>
#include <player_component.hpp>

namespace ability_controller {

void Update(entt::registry &registry) {
  registry.view<PlayerComponent, AbilityComponent, Position>().each(
      [&](PlayerComponent &player_component, AbilityComponent &ability_component,
         Position &position_component) {
        //First check if primary ability is being used
		//AND ability is not on cooldown 
        if (ability_component.use_primary && !(ability_component.cooldown > 0)) {

			TriggerAbility(registry, ability_component.primary_ability);

        }


		//Then check if secondary ability is being used
        if (ability_component.use_secondary) {

			TriggerAbility(registry, ability_component.secondary_ability);

        }

      });


}

};  // namespace ability_controller

void TriggerAbility(entt::registry &registry, AbilityID in_id) {
  
  switch (in_id) {
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

void CreateMissileEntity(entt::registry &registry) {
  // Create new entity
  auto entity = registry.create();

  // WIP: Assign it the correct components here
}

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
