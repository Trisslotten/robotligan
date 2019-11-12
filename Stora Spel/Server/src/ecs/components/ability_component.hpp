#ifndef ABILITY_COMPONENT_HPP_
#define ABILITY_COMPONENT_HPP_

#include "shared/shared.hpp"

struct AbilityComponent {
  AbilityID primary_ability = AbilityID::NULL_ABILITY;
  bool use_primary = false;
  float cooldown_max = 0.0f;
  float cooldown_remaining = 0.0f;

  AbilityID secondary_ability = AbilityID::NULL_ABILITY;
  bool use_secondary = false;

  bool shoot = false;
  float shoot_cooldown = 0.0f;

  // Comparasion Operators
  bool operator==(const AbilityComponent& rhs) {
    return (this->primary_ability == rhs.primary_ability) &&
           (this->use_primary == rhs.use_primary) &&
           (this->cooldown_max == rhs.cooldown_max) &&
           (this->cooldown_remaining == rhs.cooldown_remaining) &&
           (this->secondary_ability == rhs.secondary_ability) &&
           (this->use_secondary == rhs.use_secondary) &&
           (this->shoot == rhs.shoot) &&
           (this->shoot_cooldown == rhs.shoot_cooldown);
  }

  bool operator!=(const AbilityComponent& rhs) { return !((*this) == rhs); }
};

#endif  // !ABILITY_COMPONENT_HPP_