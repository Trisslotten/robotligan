#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <entity/registry.hpp>
#include "boundingboxes.h"
#include "ball_component.h"
#include "collision.h"
#include <glm.hpp>

// temp function to print glm::vec3
void printglm1(glm::vec3 v) {
  std::cout << "x: " << v.x << " y: " << v.y << " z: " << v.z;
}


void UpdateCollisions(entt::registry &registry) {
  //check ball collision
  auto view_ball = registry.view<BallComponent, physics::Sphere, Velocity>();
  auto view_player = registry.view<physics::OBB, Velocity>();
  auto view_arena = registry.view<physics::Arena, Velocity>();

  // Loop over all balls
  for (auto entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(entity);
    auto& s = view_ball.get<physics::Sphere>(entity);
    auto& v = view_ball.get<Velocity>(entity);

    // Collision between ball and players
    for (auto player : view_player) {
      auto& o = view_player.get<physics::OBB>(player);

      glm::vec3 normal;
      if (Intersect(s, o, &normal)) {
        //printglm1(normal);
        //std::cout << "collision" << std::endl;
        float dot_val = glm::dot(v.velocity, normal);
        v.velocity = v.velocity - normal * dot_val * 2.f;
      } else {
        //std::cout << "no collision" << std::endl;
      }

      // Collision between Player and Arena
      for (auto arena : view_arena) {
        auto& a = view_arena.get<physics::Arena>(arena);
        glm::vec3 new_pos;
        if (Intersect(a, o, &new_pos)) {
          o.center = new_pos;
        }
      }
    }

    // Collision between ball and arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get<physics::Arena>(arena);

      glm::vec3 normal;
      if (Intersect(a, s, &normal)) {
        //printglm1(normal);
        //std::cout << "collision" << std::endl;
        float dot_val = glm::dot(v.velocity, normal);
        v.velocity = v.velocity - normal * dot_val * 2.f;
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }
  }
}

#endif  // COLLISION_SYSTEM_H