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

    // Rotate OBB
    auto mat_rot = glm::toMat4(transform.rotation);
    hitbox.normals[0] = mat_rot * glm::vec4(1.0f, 0.f, 0.f, 0.f);
    hitbox.normals[2] = mat_rot * glm::vec4(0.0f, 0.f, 1.f, 0.f);
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
      auto& player_physics = view_player.get<PhysicsComponent>(player);

      glm::vec3 normal;
      float move_distance;
      if (Intersect(ball_hitbox, player_hitbox, &normal, &move_distance)) {
        //std::cout << "collision" << std::endl;
        ball_transform.position += normal * move_distance;
        float ball_speed = glm::length(ball_physics.velocity);
        float player_speed = glm::length(player_physics.velocity);
        if (ball_speed < player_speed) {
          ball_physics.velocity = normal * (glm::dot(player_physics.velocity, normal) + 1.f);
        } else {
          float dot_val = glm::dot(ball_physics.velocity, normal);
          if (dot_val < 0)
            ball_physics.velocity = ball_physics.velocity - normal * dot_val * 0.8f * 2.f;
        }
      }
    }

    // Collision between ball and arena
    for (auto arena : view_arena) {
      auto& arena_hitbox = view_arena.get(arena);

      glm::vec3 normal;
      if (Intersect(arena_hitbox, ball_hitbox, &normal)) {
        //std::cout << "collision" << std::endl;

        // Temp code to not collide with empty goal
        //auto& pos = ball_transform.position;
        //if (pos.y < -1.58 && pos.z > -6.823 && pos.z < 6.823 &&
        //    abs(pos.x) > 10 * 1.872f)
        //  continue;

        glm::vec3 temp_normal = glm::vec3(normal.x, 0.f, 0.f);
        float dot_val = glm::dot(ball_physics.velocity, temp_normal);
        ball_physics.velocity = ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;

        temp_normal = glm::vec3(0.f, normal.y, 0.f);
        dot_val = glm::dot(ball_physics.velocity, temp_normal);
        ball_physics.velocity =
            ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;

        temp_normal = glm::vec3(0.f, 0.f, normal.z);
        dot_val = glm::dot(ball_physics.velocity, temp_normal);
        ball_physics.velocity =
            ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;

        if (normal.x > 0) {
          ball_transform.position.x = arena_hitbox.xmin + ball_hitbox.radius;
        } else if (normal.x < 0) {
          ball_transform.position.x = arena_hitbox.xmax - ball_hitbox.radius;
        }

        if (normal.y > 0) {
          // Ball hits the ground
          ball_transform.position.y = arena_hitbox.ymin + ball_hitbox.radius;
          if (ball_physics.velocity.y < 3.f) {
            ball_physics.velocity.y = 0.f;
            ball_physics.is_airborne = false;
            ball.rotation = glm::quat();
          }
        } else if (normal.y < 0) {
          // Ball hits the ceiling
          ball_transform.position.y = arena_hitbox.ymax - ball_hitbox.radius;
        }

        if (normal.z > 0) {
          ball_transform.position.z = arena_hitbox.zmin + ball_hitbox.radius;
        } else if (normal.z < 0) {
          ball_transform.position.z = arena_hitbox.zmax - ball_hitbox.radius;
        }

        // Rotate ball
        glm::vec3 nn = glm::normalize(normal);
        glm::vec3 dir =
            ball_physics.velocity - glm::dot(ball_physics.velocity, nn) * nn;
        
        if (glm::length(dir) == 0) continue;
        
        glm::vec3 rotate = glm::normalize(glm::cross(nn, dir));
        float amount = glm::length(dir);
        amount *= 0.7;
        ball.rotation = glm::quat(0, rotate * amount);
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
    auto& transform_c = view_player.get<TransformComponent>(player);

    // Collision between Player and Arena
    for (auto arena : view_arena) {
      auto& a = view_arena.get(arena);
      glm::vec3 move_vector;
      if (Intersect(a, player_hitbox, &move_vector)) {
        player_hitbox.center += move_vector;
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
      auto& physics_c2 = view_player.get<PhysicsComponent>(p2);
      auto& transform_c2 = view_player.get<TransformComponent>(p2);

      glm::vec3 collision_normal;
      float distance;
      if (physics::Intersect(player_hitbox, player2_hitbox, &collision_normal, &distance)) {
        player2_hitbox.center += -collision_normal * distance;
        if (glm::dot(glm::vec3(0.f, 1.f, 0.f), collision_normal) < 0.f) {
          physics_c2.velocity.y = 0.f;
        }
        //std::cout << "normal: " << collision_normal.x << " "
        //          << collision_normal.y << " " << collision_normal.z << " " << distance
        //          << std::endl;
        glm::vec3 vel = physics_c.velocity + physics_c2.velocity;
        vel.y = 0.f;
        //player_hitbox.center += vel * 0.01f; //glm::vec3(1.f, 0.f, 0.f);
        //player2_hitbox.center -= vel * 0.01f;  // glm::vec3(1.f, 0.f, 0.f);

        transform_c.position = player_hitbox.center;
        transform_c2.position = player2_hitbox.center;
      }
    }

    // Collision between player and projectile
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);
      glm::vec3 normal;
      float dummy;
      if (Intersect(proj_hitbox, player_hitbox, &normal, &dummy)) {
        std::cout << "normal: " << normal.x << " " << normal.y << " " << normal.z
                  << std::endl;
        if (id.projectile_id == CANNON_BALL) {
          registry.destroy(projectile);
        }
      }
    }
  }
}

#endif  // COLLISION_SYSTEM_HPP_