#ifndef PROJECTILE_COMPONENT_HPP_
#define PROJECTILE_COMPONENT_HPP_

enum ProjectileID {
  CANNON_BALL
};

struct ProjectileComponent {
  ProjectileID projectile_id;
};

#endif  // !PROJECTLIE_COMPONENT_HPP_