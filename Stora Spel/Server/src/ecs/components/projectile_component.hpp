#ifndef PROJECTILE_COMPONENT_HPP_
#define PROJECTILE_COMPONENT_HPP_

#include <shared.hpp>

enum ProjectileID {
  CANNON_BALL,
  NUM_PROJECTILE_IDS,
};

struct ProjectileComponent {
  ProjectileID projectile_id;
  PlayerID creator;
};

#endif  // !PROJECTLIE_COMPONENT_HPP_