#ifndef MISSILE_COMPONENT_HPP_
#define MISSILE_COMPONENT_HPP_
#include <shared/shared.hpp>

struct MissileComponent {
  float speed = 2.f;
  EntityID target_id = -1;
  EntityID creator = -1;
  float turn_rate = 1.6f;
  float detonation_dist = 1.f;
};

#endif  // !MISSILE_COMPONENT_HPP_
