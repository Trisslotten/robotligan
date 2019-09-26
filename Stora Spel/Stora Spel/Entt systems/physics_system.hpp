#ifndef PHYSICS_SYSTEM_HPP_
#define PHYSICS_SYSTEM_HPP_

#include <iostream>

#include <entity/registry.hpp>
#include <glm/ext/quaternion_common.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "physics.hpp"
#include "physics_component.hpp"
#include "transform_component.hpp"

void UpdatePhysics(entt::registry& registry, float dt) {
  auto view_moveable = registry.view<TransformComponent, PhysicsComponent>();
  auto view_ball = registry.view<TransformComponent, PhysicsComponent, BallComponent, physics::Sphere>();

  for (auto entity : view_moveable) {
    TransformComponent& trans_c = view_moveable.get<TransformComponent>(entity);
    PhysicsComponent& physics_c = view_moveable.get<PhysicsComponent>(entity);

    physics::PhysicsObject po;
    po.airborne = physics_c.is_airborne;
    po.friction = physics_c.friction;
    po.position = trans_c.position;
    po.velocity = physics_c.velocity;

	  Update(&po, dt);

    trans_c.position = po.position;
    physics_c.velocity = po.velocity;

    // trans_c.position += physics_c.velocity;
  }


  // Rotate the ball ===========================================================================
  for (auto entity : view_ball) {
    PhysicsComponent& physics_c = view_ball.get<PhysicsComponent>(entity);
    TransformComponent& trans_c = view_ball.get<TransformComponent>(entity);
    
    if (physics_c.is_airborne == false) {
      physics::Sphere& sphere_c = view_ball.get<physics::Sphere>(entity);
      float distance = glm::length(physics_c.velocity);
      float radians = distance / sphere_c.radius;
      
      if (radians == 0.f) break;
    
      glm::vec3 direction =
          glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), physics_c.velocity));
    
    
      //glm::quat r = glm::angleAxis(radians, direction);
      direction = radians * direction;
      glm::quat r(0, direction);
      glm::quat spin = 0.5f * r * trans_c.rotation;
    
      trans_c.rotation += spin * dt;
      trans_c.rotation = glm::normalize(trans_c.rotation);
    } else {
      auto& ball_c = view_ball.get<BallComponent>(entity);
      glm::quat spin = 0.5f * ball_c.rotation * trans_c.rotation;
      trans_c.rotation += spin * dt;
      trans_c.rotation = glm::normalize(trans_c.rotation);
    }
  }
}

#endif  // PHYSICS_SYSTEM_HPP_
