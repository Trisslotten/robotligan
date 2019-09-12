#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <entity/registry.hpp>
#include "ball_component.h"
#include "boundingboxes.h"
#include "collision.h"
#include "velocity.h"
#include "physics.h"
#include "physics_component.h"


// temp function to print glm::vec3
void printglm(glm::vec3 v) {
  std::cout << "x: " << v.x << " y: " << v.y << " z: " << v.z;
}

void UpdatePhysics(entt::registry& registry, float dt) {
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, Velocity>();

  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<physics::Sphere>(entity);
    auto& v = view_ball.get<Velocity>(entity);

    physics::PhysicsObject po;
    po.airborne = ball.is_airborne;
    po.friction = .0f;
    po.position = s.center;
    po.velocity = v.velocity;
    //printglm(s.center);
    //std::cout << std::endl;

    update(&po, dt);

    s.center = po.position;
    v.velocity = po.velocity;
    //printglm(s.center);
    //std::cout << std::endl;
  }

  auto view_players = registry.view<TransformComponent, Velocity>();


}

#endif  // PHYSICS_SYSTEM_H
