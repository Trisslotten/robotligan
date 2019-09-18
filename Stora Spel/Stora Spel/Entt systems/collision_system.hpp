#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "transform_component.hpp"
#include "velocity_component.hpp"

void UpdateCollisions(entt::registry &registry) {
  auto view_ball = registry.view<BallComponent, physics::Sphere, VelocityComponent, TransformComponent>();
  auto view_player = registry.view<physics::OBB, VelocityComponent>();
  auto view_arena = registry.view<physics::Arena>();

  //check ball collision
  // Loop over all balls
  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<physics::Sphere>(entity);
    auto& v = view_ball.get<VelocityComponent>(entity);
    auto& t = view_ball.get<TransformComponent>(entity);

    // Collision between ball and players
    for (auto player : view_player) {
      auto& o = view_player.get<physics::OBB>(player);

      glm::vec3 normal;
      if (Intersect(s, o, &normal)) {
        std::cout << "collision" << std::endl;
        float dot_val = glm::dot(v.velocity, normal);
        v.velocity = v.velocity - normal * dot_val * 2.f;
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }

    // Collision between ball and arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get(arena);

      glm::vec3 normal;
      if (Intersect(a, s, &normal)) {
        std::cout << "collision" << std::endl;
        float dot_val = glm::dot(v.velocity, normal);
        
        v.velocity = v.velocity - normal * dot_val * 0.9f * 2.f;
        s.center += normal * 0.1f;
        t.position = s.center;
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }
  }

  // check player collision
  // Loop over all players
  for (auto it = view_player.begin(); it != view_player.end(); ++it) {
    auto player = *it;
    auto& o = view_player.get<physics::OBB>(player);

    // Collision between Player and Arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get(arena);
      glm::vec3 new_pos;
      if (Intersect(a, o, &new_pos)) {
        o.center = new_pos;
        std::cout << "collision" << std::endl;
      }
    }

    // Collision between player and player
    for (auto it2 = it++; it2 != view_player.end(); ++it2) {
      auto p2 = *it2;
      auto& o2 = view_player.get<physics::OBB>(p2);
      if (physics::Intersect(o, o2)) {
        o.center += glm::vec3(1.f, 0.f, 0.f);
        o2.center -= glm::vec3(1.f, 0.f, 0.f);
        std::cout << "collision" << std::endl;
      }
    }
  }
}

#endif  // COLLISION_SYSTEM_HPP_