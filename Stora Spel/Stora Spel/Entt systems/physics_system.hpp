#ifndef PHYSICS_SYSTEM_HPP_
#define PHYSICS_SYSTEM_HPP_

#include <entity/registry.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "physics.hpp"
#include "velocity_component.hpp"

void UpdatePhysics(entt::registry& registry, float dt) {
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, VelocityComponent>();

  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<physics::Sphere>(entity);
    auto& v = view_ball.get<VelocityComponent>(entity);

    physics::PhysicsObject po;
    po.airborne = ball.is_airborne;
    po.friction = .0f;
    po.position = s.center;
    po.velocity = v.velocity;
    //std::cout << std::endl;

    Update(&po, dt);

    s.center = po.position;
    v.velocity = po.velocity;
    // printglm(s.center);
    // std::cout << std::endl;
  }

  auto view_moveable = registry.view<TransformComponent, Velocity>();

  for (auto entity : view_moveable) {
    TransformComponent& tc = view_moveable.get<TransformComponent>(entity);
    Velocity& vc = view_moveable.get<Velocity>(entity);

    tc.position += vc.velocity;
  }
}

#endif  // PHYSICS_SYSTEM_HPP_
