#ifndef PHYSICS_SYSTEM_HPP
#define PHYSICS_SYSTEM_HPP

#include <entity/registry.hpp>
#include "ball_component.hpp"
#include "boundingboxes.h"
#include "collision.h"
#include "physics.h"
#include "physics_component.hpp"
#include "velocity.hpp"
#include "transform_component.hpp"

// temp function to print glm::vec3
void printglm(glm::vec3 v) {
  std::cout << "x: " << v.x << " y: " << v.y << " z: " << v.z;
}

void UpdatePhysics(entt::registry& registry, float dt) {
  auto view_ball = registry.view<BallComponent, physics::Sphere, Velocity>();

  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<physics::Sphere>(entity);
    auto& v = view_ball.get<Velocity>(entity);

    physics::PhysicsObject po;
    po.airborne = ball.is_airborne;
    po.friction = .0f;
    po.position = s.center;
    po.velocity = v.velocity;
    // printglm(s.center);
    // std::cout << std::endl;

    update(&po, dt);

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

#endif  // PHYSICS_SYSTEM_HPP
