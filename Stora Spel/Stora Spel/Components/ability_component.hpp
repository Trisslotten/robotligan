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
  unsigned int cooldown = 0; 

  AbilityID secondary_ability = NULL_ABILITY;
  bool use_secondary = false;
};

#endif  // !ABILITY_COMPONENT_HPP_