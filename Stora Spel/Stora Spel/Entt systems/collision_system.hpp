#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <glm.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "velocity_component.hpp"

void UpdateCollisions(entt::registry &registry) {
  auto view_ball = registry.view<BallComponent, physics::Sphere, VelocityComponent>();
  auto view_player = registry.view<physics::OBB, VelocityComponent>();
  auto view_arena = registry.view<physics::Arena, VelocityComponent>();

  //check ball collision
  // Loop over all balls
  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<physics::Sphere>(entity);
    auto& v = view_ball.get<VelocityComponent>(entity);

    // Collision between ball and players
    for (auto player : view_player) {
      auto& o = view_player.get<physics::OBB>(player);

      glm::vec3 normal;
      if (Intersect(s, o, &normal)) {
        //std::cout << "collision" << std::endl;
        float dot_val = glm::dot(v.velocity, normal);
        v.velocity = v.velocity - normal * dot_val * 2.f;
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }

    // Collision between ball and arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get<physics::Arena>(arena);

      glm::vec3 normal;
      if (Intersect(a, s, &normal)) {
        //std::cout << "collision" << std::endl;
        float dot_val = glm::dot(v.velocity, normal);
        v.velocity = v.velocity - normal * dot_val * 2.f;
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }
  }

  // check player collision
  // Loop over all players
  for (auto player : view_player) {
    auto& o = view_player.get<physics::OBB>(player);

    // Collision between Player and Arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get<physics::Arena>(arena);
      glm::vec3 new_pos;
      if (Intersect(a, o, &new_pos)) {
        o.center = new_pos;
      }
    }

    // Collision between player and player
    for (auto p2 : view_player) {
      if (player != p2) {
        auto& o2 = view_player.get<physics::OBB>(p2);
        if (physics::Intersect(o, o2)) {
          o.center += glm::vec3(1.f, 0.f, 0.f);
          o2.center -= glm::vec3(1.f, 0.f, 0.f);
        }
      }
    }
  }
}

#endif  // COLLISION_SYSTEM_HPP_