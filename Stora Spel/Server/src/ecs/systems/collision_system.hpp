#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <iterator>

#include <entity/registry.hpp>
#include <glm/ext.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "boundingboxes.hpp"
#include "collision.hpp"
#include "ecs/components/physics_component.hpp"
#include "ecs/components/pick_up_event.hpp"
#include "ecs/components/projectile_component.hpp"
#include "shared/transform_component.hpp"

void DestroyEntity(entt::registry& registry, entt::entity entity);
void ApplyForcePush(entt::registry& registry, glm::vec3 pos);
void ApplyForcePushOnEntity(glm::vec3 explosion_pos, glm::vec3 entity_pos,
                            PhysicsComponent &physics_c);

std::ostream& operator<<(std::ostream& o, glm::vec3 v) {
  return o << v.x << " " << v.y << " " << v.z;
}

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

void HandleBallCollisions(entt::registry& registry, const CollisionList& list,
                          entt::entity arena);
void HandleMultiBallCollision(entt::registry& registry,
                              const CollisionList& list);
void PlayerBallCollision(entt::registry& registry,
                         const CollisionObject& object, entt::entity ball,
                         entt::entity arena);
void BallArenaCollision(entt::registry& registry, const CollisionObject& object,
                        entt::entity ball);
void PlayerPlayerCollision(entt::registry& registry);
void PlayerProjectileCollision(entt::registry& registry);
void PlayerArenaCollision(entt::registry& registry);
void ProjectileBallCollision(entt::registry& registry, entt::entity ball);
void ProjectileArenaCollision(entt::registry& registry);
void PickUpPlayerCollision(entt::registry& registry);
void BallCollision(PhysicsComponent* ball, glm::vec3 normal);
void UpdateSphere(entt::registry& registry);
void UpdateOBB(entt::registry& registry);
void UpdateTransform(entt::registry& registry);

void UpdateCollisions(entt::registry& registry) {
  UpdateSphere(registry);
  UpdateOBB(registry);
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, PhysicsComponent>();
  auto view_player =
      registry.view<physics::OBB, PhysicsComponent, PlayerComponent>();
  auto view_arena_mesh = registry.view<physics::MeshHitbox>();

  entt::entity arena_entity;
  for (auto a : view_arena_mesh) arena_entity = a;

  std::vector<CollisionList> ball_collisions;
  int ball_counter = 0;
  // check ball collision
  // Loop over all balls
  for (auto ball_entity : view_ball) {
    auto& ball_hitbox = view_ball.get<physics::Sphere>(ball_entity);
    BallComponent& ball_ball = view_ball.get<BallComponent>(ball_entity);

    ball_collisions.push_back({});
    ball_collisions[ball_counter].entity = ball_entity;
    // Collision between ball and players
    for (auto player : view_player) {
      auto& player_hitbox = view_player.get<physics::OBB>(player);
      PlayerComponent& player_player = view_player.get<PlayerComponent>(player);

      physics::IntersectData data = Intersect(ball_hitbox, player_hitbox);
      if (data.collision) {
        ball_collisions[ball_counter].collision_list.push_back(
            {player, data.normal, data.move_vector, PLAYER});
        ball_ball.last_touch = player_player.client_id;
      }
    }

    // Collision between ball and arena
    for (auto arena : view_arena_mesh) {
      auto& arena_hitbox = view_arena_mesh.get(arena);
      physics::IntersectData data = Intersect(arena_hitbox, ball_hitbox);
      if (data.collision)
        ball_collisions[ball_counter].collision_list.push_back(
            {arena, data.normal, data.move_vector, ARENA});
    }

    // collision with ball and projectiles
    ProjectileBallCollision(registry, ball_entity);

    ball_counter++;
  }

  // collision with arena and projectiles
  ProjectileArenaCollision(registry);
  // check player collision
  // Collision between player and player
  PlayerPlayerCollision(registry);
  // Collision between Player and Arena
  PlayerArenaCollision(registry);
  // Collision between player and projectile
  PlayerProjectileCollision(registry);
  // Collision between player and pic-up
  PickUpPlayerCollision(registry);

  // HANDLE BALL COLLISIONS
  for (int i = 0; i < ball_collisions.size(); ++i) {
    HandleBallCollisions(registry, ball_collisions[i], arena_entity);
  }

  // NEEDS TO BE CALLED LAST
  UpdateTransform(registry);

  return;
}

void HandleBallCollisions(entt::registry& registry, const CollisionList& list,
                          entt::entity arena) {
  if (list.collision_list.size() == 1) {
    auto& object = list.collision_list[0];

    if (object.tag ==
        PLAYER) {  // the only object colliding with the ball is a player
      PlayerBallCollision(registry, object, list.entity, arena);
    } else if (object.tag ==
               ARENA) {  // the only object colliding with the ball is the arena
      BallArenaCollision(registry, object, list.entity);
    }
  } else {
    HandleMultiBallCollision(registry, list);
  }

  return;
}

void HandleMultiBallCollision(entt::registry& registry,
                              const CollisionList& list) {
  auto& ball_physics = registry.get<PhysicsComponent>(list.entity);
  auto& ball_hitbox = registry.get<physics::Sphere>(list.entity);

  for (auto obj : list.collision_list) {
    if (obj.tag == ARENA) {
      ball_hitbox.center += obj.move_vector;
      // ball_physics.velocity = glm::vec3(0.f);
      BallCollision(&ball_physics, obj.normal);

      break;
    }
  }

  for (auto obj : list.collision_list) {
    if (obj.tag == PLAYER) {
      auto& player_hitbox = registry.get<physics::OBB>(obj.entity);
      auto& player_physics = registry.get<PhysicsComponent>(obj.entity);
      physics::IntersectData data = Intersect(ball_hitbox, player_hitbox);

      if (data.collision) {
        player_hitbox.center -= data.move_vector;
        if (data.move_vector.y < 0.f) player_physics.velocity.y = 0.f;

        BallCollision(&ball_physics, data.normal);
      }
    }
  }

  return;
}

void PlayerBallCollision(entt::registry& registry,
                         const CollisionObject& object, entt::entity ball,
                         entt::entity arena) {
  auto& ball_physics = registry.get<PhysicsComponent>(ball);
  auto& ball_hitbox = registry.get<physics::Sphere>(ball);

  auto& player_physics = registry.get<PhysicsComponent>(object.entity);

  ball_hitbox.center += object.move_vector;

  float ball_speed = glm::length(ball_physics.velocity);
  float player_speed = glm::length(player_physics.velocity);
  if (ball_speed < player_speed) {
    ball_physics.velocity =
        object.normal *
        (glm::dot(player_physics.velocity, object.normal) + 1.f);

  } else {
    BallCollision(&ball_physics, object.normal);  // player_physics.velocity);
  }

  physics::IntersectData data =
      Intersect(registry.get<physics::MeshHitbox>(arena), ball_hitbox);
  if (data.collision) {
    ball_hitbox.center += data.move_vector;

    auto& player_hitbox = registry.get<physics::OBB>(object.entity);
    data = Intersect(ball_hitbox, player_hitbox);
    if (data.collision) {
      player_hitbox.center -= data.move_vector;
      auto& player_physics = registry.get<PhysicsComponent>(object.entity);

      if (data.move_vector.y < 0.f) player_physics.velocity.y = 0.f;

      BallCollision(&ball_physics, object.normal);
    }
  }

  return;
}

void BallArenaCollision(entt::registry& registry, const CollisionObject& object,
                        entt::entity ball) {
  auto& ball_physics = registry.get<PhysicsComponent>(ball);
  auto& ball_hitbox = registry.get<physics::Sphere>(ball);
  auto& ball_c = registry.get<BallComponent>(ball);

  ball_hitbox.center += object.move_vector;

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
    if (ball_physics.velocity.y < 0.8f) {
      ball_physics.velocity.y = 0.f;
      ball_physics.is_airborne = false;
      ball_c.rotation = glm::quat();
    }
  }

  // Rotate ball
  glm::vec3 nn = glm::normalize(object.normal);
  glm::vec3 dir =
      ball_physics.velocity - glm::dot(ball_physics.velocity, nn) * nn;

  if (glm::length(dir) == 0) return;

  glm::vec3 rotate = glm::normalize(glm::cross(nn, dir));
  float amount = glm::length(dir);
  amount *= 0.7;
  ball_c.rotation = glm::quat(0, rotate * amount);

  return;
}

void PlayerArenaCollision(entt::registry& registry) {
  auto view_player = registry.view<physics::OBB, PhysicsComponent>();
  auto view_arena = registry.view<physics::Arena>();

  for (auto player : view_player) {
    auto& player_hitbox = registry.get<physics::OBB>(player);
    auto& physics_c = registry.get<PhysicsComponent>(player);

    for (auto arena : view_arena) {
      auto& arena_hitbox = view_arena.get(arena);

      physics::IntersectData data = Intersect(arena_hitbox, player_hitbox);
      if (data.collision) {
        player_hitbox.center += data.move_vector;
        if (data.move_vector.y > 0.0f) {
          physics_c.is_airborne = false;
          physics_c.velocity.y = 0.f;
        }
      }
    }
  }

  return;
}

void PlayerPlayerCollision(entt::registry& registry) {
  auto view_player = registry.view<physics::OBB, PhysicsComponent>();

  for (auto it = view_player.begin(); it != view_player.end(); ++it) {
    auto player1 = *it;

    for (auto it2 = std::next(it, 1); it2 != view_player.end(); ++it2) {
      auto player2 = *it2;

      auto& player1_hitbox = registry.get<physics::OBB>(player1);
      auto& player2_hitbox = registry.get<physics::OBB>(player2);

      physics::IntersectData data = Intersect(player1_hitbox, player2_hitbox);
      if (data.collision) {
        auto& physics_c1 = registry.get<PhysicsComponent>(player1);
        auto& physics_c2 = registry.get<PhysicsComponent>(player2);

        float dot_val1 = glm::dot(physics_c1.velocity, data.normal);
        float dot_val2 = glm::dot(physics_c2.velocity, data.normal);
        if (dot_val1 == 0.f && dot_val2 == 0.f) continue;

        // running the same direction
        if ((dot_val1 > 0.f && dot_val2 > 0.f) ||
            (dot_val1 > 0.f && dot_val2 > 0.f)) {
          if (glm::dot(data.normal,
                       player1_hitbox.center - player2_hitbox.center) > 0.f) {
            dot_val1 = 0.f;
            std::cout << "move player 2\n";
          } else {
            dot_val2 = 0.f;
            std::cout << "move player 1\n";
          }
        } else if (dot_val1 == 0.f) {
          dot_val2 = 1.f;
        } else if (dot_val2 == 0.f) {
          dot_val1 = -1.f;
        }

        glm::vec3 p1_move =
            data.move_vector * dot_val1 / (fabs(dot_val1) + fabs(dot_val2));
        glm::vec3 p2_move =
            data.move_vector * dot_val2 / (fabs(dot_val1) + fabs(dot_val2));
        player1_hitbox.center -= p1_move;
        player2_hitbox.center -= p2_move;

        if (glm::dot(glm::vec3(0.f, 1.f, 0.f), data.normal) < 0.f) {
          physics_c2.velocity.y = 0.f;
        }
      }
    }
  }
}

void PlayerProjectileCollision(entt::registry& registry) {
  auto view_player = registry.view<physics::OBB, PhysicsComponent>();
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();

  for (auto player : view_player) {
    auto& player_hitbox = registry.get<physics::OBB>(player);

    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);

      physics::IntersectData data = Intersect(proj_hitbox, player_hitbox);
      if (data.collision) {
        switch (id.projectile_id) {
          case ProjectileID::CANNON_BALL: {
            // registry.destroy(projectile);
            DestroyEntity(registry, projectile);
            break;
          }
          case ProjectileID::FORCE_PUSH_OBJECT: {
            ApplyForcePush(registry, proj_hitbox.center);
            DestroyEntity(registry, projectile);
            break;
          }
        }
      }
    }
  }

  return;
}

void ProjectileBallCollision(entt::registry& registry, entt::entity ball) {
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();
  auto& ball_hitbox = registry.get<physics::Sphere>(ball);
  auto& ball_c = registry.get<BallComponent>(ball);
  auto& ball_physics = registry.get<PhysicsComponent>(ball);

  for (auto projectile : view_projectile) {
    auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
    auto& id = view_projectile.get<ProjectileComponent>(projectile);

    physics::IntersectData data = Intersect(ball_hitbox, proj_hitbox);
    if (data.collision) {
      switch (id.projectile_id) {
        case ProjectileID::CANNON_BALL: {
          glm::vec3 dir = normalize(ball_hitbox.center - proj_hitbox.center);
          ball_physics.velocity = dir * 20.0f;
          ball_physics.is_airborne = true;
          ball_c.last_touch = id.creator;
          // registry.destroy(projectile);
          DestroyEntity(registry, projectile);
          break;
        }
        case ProjectileID::FORCE_PUSH_OBJECT: {
          ApplyForcePush(registry, proj_hitbox.center);
          DestroyEntity(registry, projectile);
          break;
        }
      }
    }
  }

  return;
}

void ProjectileArenaCollision(entt::registry& registry) {
  auto view_arena = registry.view<physics::Arena>();
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();

  for (auto arena : view_arena) {
    auto& arena_hitbox = view_arena.get(arena);
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);

      physics::IntersectData data = Intersect(arena_hitbox, proj_hitbox);
      if (data.collision) {
        switch (id.projectile_id) {
          case ProjectileID::CANNON_BALL: {
            // registry.destroy(projectile);
            DestroyEntity(registry, projectile);
            break;
          }
          case ProjectileID::FORCE_PUSH_OBJECT: {
            ApplyForcePush(registry, proj_hitbox.center);
            DestroyEntity(registry, projectile);
            break;
          }
        }
      }
    }
  }

  return;
}

void PickUpPlayerCollision(entt::registry& registry) {
  auto pick_up_view = registry.view<PickUpComponent, physics::OBB>();
  auto view_player = registry.view<physics::OBB, PhysicsComponent,
                                   PlayerComponent, AbilityComponent>();

  for (auto pick_up : pick_up_view) {
    auto& pick_up_obb = pick_up_view.get<physics::OBB>(pick_up);

    for (auto player : view_player) {
      auto& player_obb = view_player.get<physics::OBB>(player);

      auto data = Intersect(pick_up_obb, player_obb);
      if (data.collision) {
        auto entity = registry.create();
        registry.assign<PickUpEvent>(
            entity, pick_up_view.get<PickUpComponent>(pick_up).id,
            view_player.get<PlayerComponent>(player).client_id,
            AbilityID::SWITCH_GOALS);

        auto& ability_c = view_player.get<AbilityComponent>(player);
        ability_c.secondary_ability = AbilityID::SWITCH_GOALS;

        registry.destroy(pick_up);
      }
    }
  }

  return;
}

void BallCollision(PhysicsComponent* ball, glm::vec3 normal) {
  float dot_val = glm::dot(ball->velocity, normal);
  if (dot_val < 0.f)
    ball->velocity = ball->velocity - normal * dot_val * 0.8f * 2.f;
}

void UpdateSphere(entt::registry& registry) {
  auto view_moveable = registry.view<TransformComponent, physics::Sphere>();
  for (auto object : view_moveable) {
    auto& hitbox = view_moveable.get<physics::Sphere>(object);
    auto& transform = view_moveable.get<TransformComponent>(object);
    hitbox.center = transform.position;
  }
}

void UpdateOBB(entt::registry& registry) {
  auto view_moveable = registry.view<TransformComponent, physics::OBB>();
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

void UpdateTransform(entt::registry& registry) {
  auto view_sphere = registry.view<TransformComponent, physics::Sphere>();
  for (auto sphere : view_sphere) {
    auto& transform = view_sphere.get<TransformComponent>(sphere);
    auto& hitbox = view_sphere.get<physics::Sphere>(sphere);

    transform.position = hitbox.center;
  }

  auto view_obb = registry.view<TransformComponent, physics::OBB>();
  for (auto obb : view_obb) {
    auto& transform = view_obb.get<TransformComponent>(obb);
    auto& hitbox = view_obb.get<physics::OBB>(obb);

    transform.position = hitbox.center;
  }
}

void ApplyForcePush(entt::registry& registry, glm::vec3 pos) {
  physics::Sphere force_push;
  force_push.center = pos;
  force_push.radius =
      GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_RADIUS");

  auto balls =
      registry.view<physics::Sphere, BallComponent, PhysicsComponent>();
  for (auto ball : balls) {
    auto& hitbox = balls.get<physics::Sphere>(ball);
    auto& physics_c = balls.get<PhysicsComponent>(ball);
    ApplyForcePushOnEntity(force_push.center, hitbox.center, physics_c);
  }

  auto players =
      registry.view<physics::OBB, PlayerComponent, PhysicsComponent>();

   for (auto player : players) {
    auto& hitbox = players.get<physics::OBB>(player);
     auto& physics_c = players.get<PhysicsComponent>(player);
    ApplyForcePushOnEntity(force_push.center, hitbox.center, physics_c);
  }

}

void ApplyForcePushOnEntity(glm::vec3 explosion_pos, glm::vec3 entity_pos, PhysicsComponent& physics_c) {
  physics::Sphere force_push;
  force_push.center = explosion_pos;
  force_push.radius =
      GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_RADIUS");
  glm::vec3 dir = entity_pos - force_push.center;
  float length = glm::length(dir);
  if (length < force_push.radius) {
    physics_c.is_airborne = true;
    float force =
        GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_STRENGTH");
    physics_c.velocity =
        dir * force * (force_push.radius - length) / force_push.radius;
  }
}

void DestroyEntity(entt::registry& registry, entt::entity entity) {
  EventInfo info;
  if (registry.has<IDComponent>(entity) == false) return;
  auto id = registry.get<IDComponent>(entity);
  info.event = Event::DESTROY_ENTITY;
  info.e_id = id.id;
  dispatcher.enqueue<EventInfo>(info);
  registry.destroy(entity);
}
#endif  // COLLISION_SYSTEM_HPP_