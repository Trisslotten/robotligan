#ifndef ABILITY_COMPONENT_HPP_
#define ABILITY_COMPONENT_HPP_

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

struct AbilityComponent {
  AbilityID primary_ability = NULL_ABILITY;
  bool use_primary = false;
  float cooldown = 0.0f;

  AbilityID secondary_ability = NULL_ABILITY;
  bool use_secondary = false;

  bool shoot = false;
  float shoot_cooldown = 0.0f;
};

#endif  // !ABILITY_COMPONENT_HPP_