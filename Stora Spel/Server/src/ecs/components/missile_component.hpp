#ifndef MISSILE_COMPONENT_HPP_
#define MISSILE_COMPONENT_HPP_
#include <shared/shared.hpp>

struct MissileComponent {
  float speed = 5;
  EntityID target_id = -1;
};

#endif  // !MISSILE_COMPONENT_HPP_
