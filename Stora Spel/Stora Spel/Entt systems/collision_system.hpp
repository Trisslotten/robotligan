#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <iterator>

#include <entity/registry.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <glm/ext.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "projectile_component.hpp"
#include "physics_component.hpp"
#include "transform_component.hpp"

enum ObjectTag { PLAYER, BALL, ARENA, PROJECTLIE };
struct CollisionObject {
  entt::entity entity;
  glm::vec3 normal;
  glm::vec3 move_vector;
  ObjectTag tag;
};

struct CollisionList {
  entt::entity entity;
  std::vector<CollisionObject> collision_list;
};

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
  auto view_arena_mesh = registry.view<physics::MeshHitbox>();
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();

  int nr_of_balls = view_ball.size();
  CollisionList* ball_collisions = new CollisionList[nr_of_balls];
  int ball_counter = 0;
  //check ball collision
  // Loop over all balls
  for (auto ball_entity : view_ball) {
    auto& ball = view_ball.get<BallComponent>(ball_entity);
    auto& ball_hitbox = view_ball.get<physics::Sphere>(ball_entity);
    auto& ball_physics = view_ball.get<PhysicsComponent>(ball_entity);
    auto& ball_transform = view_ball.get<TransformComponent>(ball_entity);

	ball_collisions[ball_counter].entity = ball_entity;
    // Collision between ball and players
    for (auto player : view_player) {
      auto& player_hitbox = view_player.get<physics::OBB>(player);
      auto& player_physics = view_player.get<PhysicsComponent>(player);

      physics::IntersectData data = Intersect(ball_hitbox, player_hitbox);
      if (data.collision) {
        ball_collisions[ball_counter].collision_list.push_back(
            {player, data.normal, data.move_vector, PLAYER});
        //ball_transform.position += data.move_vector;
        //float ball_speed = glm::length(ball_physics.velocity);
        //float player_speed = glm::length(player_physics.velocity);
        //if (ball_speed < player_speed) {
        //  ball_physics.velocity = data.normal * (glm::dot(player_physics.velocity, data.normal) + 1.f);
        //} else {
        //  float dot_val = glm::dot(ball_physics.velocity, data.normal);
        //  if (dot_val < 0)
        //    ball_physics.velocity = ball_physics.velocity - data.normal * dot_val * 0.8f * 2.f;
        //}
      }
    }

    // Collision between ball and arena mesh
    for (auto arena : view_arena_mesh) {
      auto& arena_hitbox = view_arena_mesh.get(arena);
      physics::IntersectData data = Intersect(arena_hitbox, ball_hitbox);
      if (data.collision) {
        ball_collisions[ball_counter].collision_list.push_back(
            {arena, data.normal, data.move_vector, ARENA});
       // std::cout << "mesh hitbox: " << data.normal.x << " " << data.normal.y
       //           << " " << data.normal.z << std::endl;
	   //
       // ball_transform.position += data.move_vector;
	   //
       // if (data.normal.x) {
       //   glm::vec3 temp_normal =
       //       glm::normalize(glm::vec3(data.normal.x, 0.f, 0.f));
       //   float dot_val = glm::dot(ball_physics.velocity, temp_normal);
       //   if (dot_val < 0.f)
       //     ball_physics.velocity =
       //         ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
       // }
	   //
       // if (data.normal.y) {
       //   glm::vec3 temp_normal =
       //       glm::normalize(glm::vec3(0.f, data.normal.y, 0.f));
       //   float dot_val = glm::dot(ball_physics.velocity, temp_normal);
       //   if (dot_val < 0.f)
       //     ball_physics.velocity =
       //         ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
       // }
	   //
       // if (data.normal.z) {
       //   glm::vec3 temp_normal =
       //       glm::normalize(glm::vec3(0.f, 0.f, data.normal.z));
       //   float dot_val = glm::dot(ball_physics.velocity, temp_normal);
       //   if (dot_val < 0.f)
       //     ball_physics.velocity =
       //         ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
       // }

        // glm::vec3 temp_normal =
        //    glm::normalize(data.normal);
        // float dot_val = glm::dot(ball_physics.velocity, temp_normal);
        // ball_physics.velocity =
        //    ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;

      //  if (data.normal.y > 0.2f) {
      //    // Ball hits the ground
      //    if (ball_physics.velocity.y < 3.f) {
      //      ball_physics.velocity.y = 0.f;
      //      ball_physics.is_airborne = false;
      //      ball.rotation = glm::quat();
      //    }
      //  }
	  //
      //  // Rotate ball
      //  glm::vec3 nn = glm::normalize(data.normal);
      //  glm::vec3 dir =
      //      ball_physics.velocity - glm::dot(ball_physics.velocity, nn) * nn;
	  //
      //  if (glm::length(dir) == 0) continue;
	  //
      //  glm::vec3 rotate = glm::normalize(glm::cross(nn, dir));
      //  float amount = glm::length(dir);
      //  amount *= 0.7;
      //  ball.rotation = glm::quat(0, rotate * amount);
      }
    }

    // Collision between ball and arena

    //for (auto arena : view_arena) {
    //  auto& arena_hitbox = view_arena.get(arena);
    //
    //  //glm::vec3 normal;
    //  physics::IntersectData data = Intersect(arena_hitbox, ball_hitbox);
    //  //if (Intersect(arena_hitbox, ball_hitbox, &normal)) {
    //  if (data.collision) {
    //    //std::cout << "arena hitbox: " << normal.x << " " << normal.y << " " << normal.z
    //    //          << std::endl;
    //    //std::cout << "collision" << std::endl;
    //
    //    // Temp code to not collide with empty goal
    //    //auto& pos = ball_transform.position;
    //    //if (pos.y < -1.58 && pos.z > -6.823 && pos.z < 6.823 &&
    //    //    abs(pos.x) > 10 * 1.872f)
    //    //  continue;
    //
    //    glm::vec3 temp_normal = glm::vec3(data.normal.x, 0.f, 0.f);
    //    float dot_val = glm::dot(ball_physics.velocity, temp_normal);
    //    ball_physics.velocity = ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
    //
    //    temp_normal = glm::vec3(0.f, data.normal.y, 0.f);
    //    dot_val = glm::dot(ball_physics.velocity, temp_normal);
    //    ball_physics.velocity =
    //        ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
    //
    //    temp_normal = glm::vec3(0.f, 0.f, data.normal.z);
    //    dot_val = glm::dot(ball_physics.velocity, temp_normal);
    //    ball_physics.velocity =
    //        ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
    //
    //    if (data.normal.x > 0) {
    //      ball_transform.position.x = arena_hitbox.xmin + ball_hitbox.radius;
    //    } else if (data.normal.x < 0) {
    //      ball_transform.position.x = arena_hitbox.xmax - ball_hitbox.radius;
    //    }
    //
    //    if (data.normal.y > 0) {
    //      // Ball hits the ground
    //      ball_transform.position.y = arena_hitbox.ymin + ball_hitbox.radius;
    //      if (ball_physics.velocity.y < 3.f) {
    //        ball_physics.velocity.y = 0.f;
    //        ball_physics.is_airborne = false;
    //      } 
    //    } else if (data.normal.y < 0) {
    //      ball_transform.position.y = arena_hitbox.ymax - ball_hitbox.radius;
    //    }
    //
    //    if (data.normal.z > 0) {
    //      ball_transform.position.z = arena_hitbox.zmin + ball_hitbox.radius;
    //    } else if (data.normal.z < 0) {
    //      ball_transform.position.z = arena_hitbox.zmax - ball_hitbox.radius;
    //    }
    //  } else {
    //    //std::cout << "no collision" << std::endl;
    //  }
    //}

	//collision with ball and projectiles
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);
      physics::IntersectData data = Intersect(ball_hitbox, proj_hitbox);
      if (data.collision) {
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
    ball_counter++;
  }

  // collision with arena and projectiles
  for (auto arena : view_arena) {
    auto& arena_hitbox = view_arena.get(arena);
	  for (auto projectile : view_projectile) {
	    auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
	    auto& id = view_projectile.get<ProjectileComponent>(projectile);
      
      physics::IntersectData data = Intersect(arena_hitbox, proj_hitbox);
      if (data.collision) {
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
      
      physics::IntersectData data = Intersect(a, player_hitbox);
      if (data.collision) {
        player_hitbox.center += data.move_vector;
        transform_c.position = player_hitbox.center;
        if (data.move_vector.y > 0.0f) {
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

      physics::IntersectData data = Intersect(player_hitbox, player2_hitbox);
      if (data.collision) {
        player2_hitbox.center += -data.normal * glm::length(data.move_vector);
        if (glm::dot(glm::vec3(0.f, 1.f, 0.f), data.normal) < 0.f) {
          physics_c2.velocity.y = 0.f;
        }
        
        glm::vec3 vel = physics_c.velocity + physics_c2.velocity;
        vel.y = 0.f;

        transform_c.position = player_hitbox.center;
        transform_c2.position = player2_hitbox.center;
      }
    }

    // Collision between player and projectile
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);
      
      physics::IntersectData data = Intersect(proj_hitbox, player_hitbox);
      if (data.collision) {
        if (id.projectile_id == CANNON_BALL) {
          registry.destroy(projectile);
        }
      }
    }
  }

  for (int i = 0; i < nr_of_balls; ++i) {
    auto& ball_transform =
        view_ball.get<TransformComponent>(ball_collisions[i].entity);
    auto& ball_physics =
        view_ball.get<PhysicsComponent>(ball_collisions[i].entity);
    auto& ball_hitbox =
        view_ball.get<physics::Sphere>(ball_collisions[i].entity);
    if (ball_collisions[i].collision_list.size() == 1) {
      auto& object = ball_collisions[i].collision_list[0];
      if (ball_collisions[i].collision_list[0].tag == PLAYER) {  // the only object colliding with the ball is a player
        auto& player_physics = view_player.get<PhysicsComponent>(object.entity);
        ball_transform.position += object.move_vector;
         float ball_speed = glm::length(ball_physics.velocity);
         float player_speed = glm::length(player_physics.velocity);
         if (ball_speed < player_speed) {
          ball_physics.velocity = object.normal * (glm::dot(player_physics.velocity, object.normal) + 1.f);
        } else {
          float dot_val = glm::dot(ball_physics.velocity, object.normal);
          if (dot_val < 0)
            ball_physics.velocity = ball_physics.velocity - object.normal * dot_val * 0.8f * 2.f;
        }
      } else if (ball_collisions[i].collision_list[0].tag == ARENA) {  // the only object colliding with the ball is the arena
        auto& ball = view_ball.get<BallComponent>(ball_collisions[i].entity);
        ball_transform.position += object.move_vector;

        if (object.normal.x) {
          glm::vec3 temp_normal =
              glm::normalize(glm::vec3(object.normal.x, 0.f, 0.f));
          float dot_val = glm::dot(ball_physics.velocity, temp_normal);
          if (dot_val < 0.f)
            ball_physics.velocity =
                ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
        }

        if (object.normal.y) {
          glm::vec3 temp_normal =
              glm::normalize(glm::vec3(0.f, object.normal.y, 0.f));
          float dot_val = glm::dot(ball_physics.velocity, temp_normal);
          if (dot_val < 0.f)
            ball_physics.velocity =
                ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
        }

        if (object.normal.z) {
          glm::vec3 temp_normal =
              glm::normalize(glm::vec3(0.f, 0.f, object.normal.z));
          float dot_val = glm::dot(ball_physics.velocity, temp_normal);
          if (dot_val < 0.f)
            ball_physics.velocity =
                ball_physics.velocity - temp_normal * dot_val * 0.8f * 2.f;
        }

		if (object.normal.y > 0.2f) {
          // Ball hits the ground
          if (ball_physics.velocity.y < 3.f) {
            ball_physics.velocity.y = 0.f;
            ball_physics.is_airborne = false;
            ball.rotation = glm::quat();
          }
        }

        // Rotate ball
        glm::vec3 nn = glm::normalize(object.normal);
        glm::vec3 dir =
            ball_physics.velocity - glm::dot(ball_physics.velocity, nn) * nn;

        if (glm::length(dir) == 0) continue;

        glm::vec3 rotate = glm::normalize(glm::cross(nn, dir));
        float amount = glm::length(dir);
        amount *= 0.7;
        ball.rotation = glm::quat(0, rotate * amount);
      }
    } else {
      // std::cout << ball_collisions.size() << std::endl;
      for (auto obj : ball_collisions[i].collision_list) {
        if (obj.tag == ARENA) {
          ball_transform.position += obj.move_vector;
          ball_physics.velocity = glm::vec3(0.f);
          break;
        }
      }
      for (auto obj : ball_collisions[i].collision_list) {
        if (obj.tag == PLAYER) {
          auto& player_hitbox = view_player.get<physics::OBB>(obj.entity);
          auto& player_transform = view_player.get<TransformComponent>(obj.entity);
          physics::IntersectData data = Intersect(ball_hitbox, player_hitbox);

		  if (data.collision)
          player_transform.position -= data.move_vector;
        }
      }
    }
  }
}

#endif  // COLLISION_SYSTEM_HPP_