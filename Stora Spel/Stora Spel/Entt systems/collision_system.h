#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <entity/registry.hpp>
#include "boundingboxes.h"
#include "ball_component.h"
#include "collision.h"

void UpdateCollisions(entt::registry &registry) {
  //check ball collision
  auto view_ball = registry.view<BallComponent, Sphere, Velocity>();
  auto view_player = registry.view<OBB, Velocity>();

  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<Sphere>(entity);
    auto& v = view_ball.get<Velocity>(entity);

    for (auto player : view_player) {
      auto& o = view_player.get<OBB>(player);

      if (Intersect(s, o)) std::cout << "collision" << std::endl;
    }
  }
}

#endif  // COLLISION_SYSTEM_H