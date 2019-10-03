#ifndef ABILITY_COMPONENT_HPP_
#define ABILITY_COMPONENT_HPP_

#include <shared.hpp>

struct AbilityComponent {
  AbilityID primary_ability = AbilityID::NULL_ABILITY;
  bool use_primary = false;
  float cooldown_max = 0.0f;
  float cooldown_remaining = 0.0f;

  AbilityID secondary_ability = AbilityID::NULL_ABILITY;
  bool use_secondary = false;

  bool shoot = false;
  float shoot_cooldown = 0.0f;
};

#endif  // !ABILITY_COMPONENT_HPP_