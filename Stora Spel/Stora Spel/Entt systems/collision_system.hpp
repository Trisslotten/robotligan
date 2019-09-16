#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "projectile_component.hpp"
#include "physics_component.hpp"

void UpdateCollisions(entt::registry &registry) {
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, PhysicsComponent>();
  auto view_player = registry.view<physics::OBB, PhysicsComponent>();
  auto view_arena = registry.view<physics::Arena, PhysicsComponent>();
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();

  //check ball collision
  // Loop over all balls
  for (auto ball_entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(ball_entity);
    auto& s = view_ball.get<physics::Sphere>(ball_entity);
    auto& v = view_ball.get<PhysicsComponent>(ball_entity);

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

	//collision with ball and projectiles
    for (auto projectile : view_projectile) {
      auto hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto id = view_projectile.get<ProjectileComponent>(projectile);
      if (Intersect(s, hitbox)) {
        if (id.projectile_id == CANNON_BALL && ball.is_real == true) {
          glm::vec3 dir = normalize(s.center - hitbox.center);
          registry.destroy(projectile);
		} else {
          registry.destroy(projectile);
          registry.destroy(ball_entity);
		}
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