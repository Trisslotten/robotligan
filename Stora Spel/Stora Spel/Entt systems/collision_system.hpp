#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <iterator>

#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "projectile_component.hpp"
#include "physics_component.hpp"
#include "transform_component.hpp"

void UpdateSphere(entt::registry& registry) {
  auto view_moveable = registry.view<TransformComponent, PhysicsComponent, physics::Sphere>();
  for (auto object : view_moveable) {
    auto& hitbox = view_moveable.get<physics::Sphere>(object);
    auto& transform = view_moveable.get<TransformComponent>(object);
    hitbox.center = transform.position;
  }
}
void UpdateOBB(entt::registry& registry) {
  auto view_moveable = registry.view<TransformComponent, PhysicsComponent, physics::OBB>();
  for (auto object : view_moveable) {
    auto& hitbox = view_moveable.get<physics::OBB>(object);
    auto& transform = view_moveable.get<TransformComponent>(object);
    hitbox.center = transform.position;

    // TODO: rotate hitbox
  }
}

void UpdateCollisions(entt::registry& registry) {
  UpdateSphere(registry);
  UpdateOBB(registry);
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, PhysicsComponent, TransformComponent>();
  auto view_player = registry.view<physics::OBB, PhysicsComponent, TransformComponent>();
  auto view_arena = registry.view<physics::Arena>();
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();
  //check ball collision
  // Loop over all balls
  for (auto ball_entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(ball_entity);
    auto& ball_hitbox = view_ball.get<physics::Sphere>(ball_entity);
    auto& ball_physics = view_ball.get<PhysicsComponent>(ball_entity);
    auto& ball_transform = view_ball.get<TransformComponent>(ball_entity);

    // Collision between ball and players
    for (auto player : view_player) {
      auto& player_hitbox = view_player.get<physics::OBB>(player);

      glm::vec3 normal;
      if (Intersect(ball_hitbox, player_hitbox, &normal)) {
        //std::cout << "collision" << std::endl;
        float dot_val = glm::dot(ball_physics.velocity, normal);
        if (dot_val < 0)
          ball_physics.velocity = ball_physics.velocity - normal * dot_val * 2.f;
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }

    // Collision between ball and arena
    for (auto arena : view_arena) {
      auto& arena_hitbox = view_arena.get(arena);

      glm::vec3 normal;
      if (Intersect(arena_hitbox, ball_hitbox, &normal)) {
        //std::cout << "collision" << std::endl;
        float dot_val = glm::dot(ball_physics.velocity, normal);
        ball_physics.velocity = ball_physics.velocity - normal * dot_val * 0.8f * 2.f;

        if (normal.x > 0) {
          ball_transform.position.x = arena_hitbox.xmin + ball_hitbox.radius;
        } else if (normal.x < 0) {
          ball_transform.position.x = arena_hitbox.xmax - ball_hitbox.radius;
        }

        if (normal.y > 0) {
          ball_transform.position.y = arena_hitbox.ymin + ball_hitbox.radius;
        } else if (normal.y < 0) {
          ball_transform.position.y = arena_hitbox.ymax - ball_hitbox.radius;
        }

        if (normal.z > 0) {
          ball_transform.position.z = arena_hitbox.zmin + ball_hitbox.radius;
        } else if (normal.z < 0) {
          ball_transform.position.z = arena_hitbox.zmax - ball_hitbox.radius;
        }
      } else {
        //std::cout << "no collision" << std::endl;
      }
    }

	//collision with ball and projectiles
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);
      if (Intersect(ball_hitbox, proj_hitbox)) {
        if (id.projectile_id == CANNON_BALL && ball.is_real == true) {
          glm::vec3 dir = normalize(ball_hitbox.center - proj_hitbox.center);
          ball_physics.velocity = dir * 20.0f;
          ball_physics.is_airborne = true;
          registry.destroy(projectile);
		} else {
          registry.destroy(projectile);
          registry.destroy(ball_entity);
		}
	  }
	}
  }

  // collision with arena and projectiles
  for (auto arena : view_arena) {
    auto& arena_hitbox = view_arena.get(arena);
	for (auto projectile : view_projectile) {
	  auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
	  auto& id = view_projectile.get<ProjectileComponent>(projectile);
      glm::vec3 normal;
      if (Intersect(arena_hitbox, proj_hitbox, &normal)) {
        if (id.projectile_id == CANNON_BALL) {
          registry.destroy(projectile);
        }
	  }
	}
  }

  // check player collision
  // Loop over all players
  for (auto it = view_player.begin(); it != view_player.end(); ++it) {
    auto player = *it;
    auto& player_hitbox = view_player.get<physics::OBB>(player);
    auto& physics_c = view_player.get<PhysicsComponent>(player);

    // Collision between Player and Arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get(arena);
      glm::vec3 move_vector;
      if (Intersect(a, player_hitbox, &move_vector)) {
        player_hitbox.center += move_vector;
        auto& transform_c = view_player.get<TransformComponent>(player);
        transform_c.position = player_hitbox.center;
        if (move_vector.y > 0.0f) {
          physics_c.is_airborne = false;
          physics_c.velocity.y = 0.f;
        }
      }
    }

    // Collision between player and player
    for (auto it2 = std::next(it, 1); it2 != view_player.end(); ++it2) {
      auto p2 = *it2;
      auto& player2_hitbox = view_player.get<physics::OBB>(p2);
      if (physics::Intersect(player_hitbox, player2_hitbox)) {
        player_hitbox.center += glm::vec3(1.f, 0.f, 0.f);
        player2_hitbox.center -= glm::vec3(1.f, 0.f, 0.f);
      }
    }
  }
}

#endif  // COLLISION_SYSTEM_HPP_