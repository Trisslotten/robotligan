#ifndef ABILITY_CONTROLLER_SYSTEM_HPP_
#define ABILITY_CONTROLLER_SYSTEM_HPP_

#include "../util/ability.hpp"

namespace ability_controller {

void Update(entt::registry &registry) {
  // Fetch all entities with the Build Wall component
  // and loop over each entity in that view

  // Fetch all entities with the Fake Ball component
  // and loop over each entity in that view

  // Fetch all entities with the Force Push component
  // and loop over each entity in that view

  // Fetch all entities with the Gravity Change component
  // and loop over each entity in that view

  // Fetch all entities with the Homing Ball component
  // and loop over each entity in that view

  // Fetch all entities with the Invisibility component
  // and loop over each entity in that view

  // Fetch all entities with the Missile component
  // and loop over each entity in that view
  auto missile_view = registry.view<MissileComponent>();
  for (auto entity : missile_view) {
    // NTS: Call function to handle missile behaviour here
  }

  // Fetch all entities with the Super Strike component
  // and loop over each entity in that view

  // Fetch all entities with the Switch Goals component
  // and loop over each entity in that view

  // Fetch all entities with the Teleport component
  // and loop over each entity in that view
}

};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
