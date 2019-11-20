#ifndef COLLISION_SYSTEM_HPP_
#define COLLISION_SYSTEM_HPP_

#include <iterator>

#include <entity/registry.hpp>
#include <glm/ext.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <shared/physics_component.hpp>
#include <vector>

#include "boundingboxes.hpp"
#include "collision.hpp"
#include "ecs/components.hpp"
#include "ecs/components/pick_up_event.hpp"
#include "ecs/systems/missile_system.hpp"
#include "shared/fail_safe_arena.hpp"
#include "shared/id_component.hpp"
#include "shared/projectile_component.hpp"
#include "shared/transform_component.hpp"

void DestroyEntity(entt::registry& registry, entt::entity entity);
void ApplyForcePush(entt::registry& registry, glm::vec3 pos);
void ApplyForcePushOnEntity(glm::vec3 explosion_pos, glm::vec3 entity_pos,
                            PhysicsComponent& physics_c);
void ApplyMineStun(PhysicsComponent& physics_c);
void TeleportToCollision(entt::registry& registry, glm::vec3 hit_pos,
                         long player_id);
void EndHomingBall(entt::registry& registry, entt::entity& in_ball);

std::ostream& operator<<(std::ostream& o, glm::vec3 v) {
  return o << v.x << " " << v.y << " " << v.z;
}

enum ObjectTag { PLAYER, BALL, ARENA, PROJECTLIE, WALL };
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
                              const CollisionList& list, entt::entity arena);
void PlayerBallCollision(entt::registry& registry,
                         const CollisionObject& object, entt::entity ball,
                         entt::entity arena);
void BallArenaCollision(entt::registry& registry, const CollisionObject& object,
                        entt::entity ball);
void BallWallCollision(entt::registry& registry, const CollisionObject& object,
                       entt::entity ball);
void BallBallCollision(entt::registry& registry);
void PlayerPlayerCollision(entt::registry& registry);
void PlayerProjectileCollision(entt::registry& registry);
void PlayerArenaCollision(entt::registry& registry);
void PlayerWallCollision(entt::registry& registry);
void ProjectileBallCollision(entt::registry& registry, entt::entity ball);
void ProjectileArenaCollision(entt::registry& registry);
void PickUpPlayerCollision(entt::registry& registry);
void MinePlayerCollision(entt::registry& registry);
void BallCollision(PhysicsComponent* ball, glm::vec3 normal);
void SetJumpToFalse(entt::registry& reg);
void UpdateSphere(entt::registry& registry);
void UpdateOBB(entt::registry& registry);
void UpdateTransform(entt::registry& registry);

void UpdateCollisions(entt::registry& registry) {
  UpdateSphere(registry);
  UpdateOBB(registry);
  SetJumpToFalse(registry);
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, PhysicsComponent>();
  auto view_player =
      registry.view<physics::OBB, PhysicsComponent, PlayerComponent>();
  auto view_arena =
      registry
          .view<physics::MeshHitbox, physics::Arena, FailSafeArenaComponent>();
  auto view_wall = registry.view<physics::OBB, WallComponent>();

  entt::entity arena_entity;
  for (auto a : view_arena) arena_entity = a;

  std::vector<CollisionList> ball_collisions;
  int ball_counter = 0;
  // check ball collision
  // Loop over all balls
  for (auto ball_entity : view_ball) {
    auto& ball_hitbox = view_ball.get<physics::Sphere>(ball_entity);
    BallComponent& ball_ball = view_ball.get<BallComponent>(ball_entity);
    auto& ball_physics = view_ball.get<PhysicsComponent>(ball_entity);

    ball_collisions.push_back({});
    ball_collisions[ball_counter].entity = ball_entity;
    // Collision between ball and arena
    for (auto arena : view_arena) {
      auto& arena_mesh_hitbox = view_arena.get<physics::MeshHitbox>(arena);
      physics::IntersectData data = Intersect(arena_mesh_hitbox, ball_hitbox);
      if (data.collision) {
        ball_collisions[ball_counter].collision_list.push_back(
            {arena, data.normal, data.move_vector, ARENA});
        EndHomingBall(registry, ball_entity);
        ball_ball.homer_cid = -1;
        // std::cout << "x: " << data.normal.x << " y: " << data.normal.y
        //          << " z: " << data.normal.z << std::endl;
      } else {
        auto& arena_hitbox = view_arena.get<FailSafeArenaComponent>(arena);
        data = Intersect(arena_hitbox.arena, ball_hitbox);
        if (data.collision) {
          EndHomingBall(registry, ball_entity);
          ball_ball.homer_cid = -1;
          ball_collisions[ball_counter].collision_list.push_back(
              {arena, data.normal, data.move_vector, ARENA});
        }
      }
    }

    // Collision between ball and wall
    for (auto wall : view_wall) {
      auto& wall_hitbox = view_wall.get<physics::OBB>(wall);

      if (glm::dot(ball_physics.velocity,
                   ball_hitbox.center - wall_hitbox.center) < 0.f) {
        glm::vec3 step = ball_physics.velocity * 1.0f / 64.f;
        ball_hitbox.center -= step;
        step *= 0.1;
        for (int i = 0; i < 10; ++i) {
          physics::IntersectData data = Intersect(ball_hitbox, wall_hitbox);

          if (data.collision) {
            EndHomingBall(registry, ball_entity);
            ball_collisions[ball_counter].collision_list.push_back(
                {wall, data.normal, data.move_vector, WALL});
            break;
          }
          ball_hitbox.center += step;
        }
      } else {
        physics::IntersectData data = Intersect(ball_hitbox, wall_hitbox);

        if (data.collision) {
          EndHomingBall(registry, ball_entity);
          ball_collisions[ball_counter].collision_list.push_back(
              {wall, data.normal, data.move_vector, WALL});
        }
      }
    }

    // Collision between ball and players
    for (auto player : view_player) {
      auto& player_hitbox = view_player.get<physics::OBB>(player);
      PlayerComponent& player_player = view_player.get<PlayerComponent>(player);
      physics::IntersectData data = Intersect(ball_hitbox, player_hitbox);
      if (data.collision) {
        ball_collisions[ball_counter].collision_list.push_back(
            {player, data.normal, data.move_vector, PLAYER});
        ball_ball.prev_touch = ball_ball.last_touch;
        ball_ball.last_touch = player_player.client_id;

        EndHomingBall(registry, ball_entity);
        ball_ball.homer_cid = -1;
        // missile_system::SetBallsAreHoming(false);
      }
    }

    // collision with ball and projectiles
    ProjectileBallCollision(registry, ball_entity);

    ball_counter++;
  }

  // collision with arena and projectiles
  ProjectileArenaCollision(registry);

  // check player collision
  // Collision between player and projectile
  PlayerProjectileCollision(registry);
  // Collision between player and player
  PlayerPlayerCollision(registry);
  // Collision between Player and Arena
  PlayerArenaCollision(registry);
  // Collision between Player and Wall
  PlayerWallCollision(registry);
  // Collision between player and pick-up
  PickUpPlayerCollision(registry);
  // Collision between player and mine
  MinePlayerCollision(registry);

  // HANDLE BALL COLLISIONS
  for (int i = 0; i < ball_collisions.size(); ++i) {
    HandleBallCollisions(registry, ball_collisions[i], arena_entity);
  }
  BallBallCollision(registry);

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
    } else if (object.tag == WALL) {
      BallWallCollision(registry, object, list.entity);

      auto& ball_physics = registry.get<PhysicsComponent>(list.entity);
      float vel = glm::length(ball_physics.velocity);
      if (vel > 20.f) {
        auto& health = registry.get<HealthComponent>(object.entity);
        health.health -= 50;
      } else if (vel > 10) {
        auto& health = registry.get<HealthComponent>(object.entity);
        health.health -= 30;
      } else if (vel > 6) {
        auto& health = registry.get<HealthComponent>(object.entity);
        health.health -= 10;
      }
    }
  } else if (list.collision_list.size() > 1) {
    HandleMultiBallCollision(registry, list, arena);
  }
  auto& ball_ball_c = registry.get<BallComponent>(list.entity);
  if (ball_ball_c.should_be_destroyed) {
    DestroyEntity(registry, list.entity);
  }
  return;
}

void HandleMultiBallCollision(entt::registry& registry,
                              const CollisionList& list, entt::entity arena) {
  auto& ball_physics = registry.get<PhysicsComponent>(list.entity);
  auto& ball_hitbox = registry.get<physics::Sphere>(list.entity);

  for (auto obj : list.collision_list) {
    if (obj.tag == PLAYER) {
      PlayerBallCollision(registry, obj, list.entity, arena);
    }
  }

  for (auto obj : list.collision_list) {
    if (obj.tag == ARENA) {
      ball_hitbox.center += obj.move_vector;
      // ball_physics.velocity = glm::vec3(0.f);
      BallCollision(&ball_physics, obj.normal);

      // break;
    } else if (obj.tag == WALL) {
      ball_hitbox.center += obj.move_vector;
      BallCollision(&ball_physics, obj.normal);
    }
  }

  for (auto obj : list.collision_list) {
    if (obj.tag == PLAYER) {
      auto& player_hitbox = registry.get<physics::OBB>(obj.entity);
      auto& player_physics = registry.get<PhysicsComponent>(obj.entity);
      physics::IntersectData data = Intersect(ball_hitbox, player_hitbox);

      if (data.collision) {
        player_hitbox.center -= data.move_vector;
        if (data.move_vector.y < 0.f) {
          player_physics.velocity.y = 0.f;
          auto& player_c = registry.get<PlayerComponent>(obj.entity);
          player_c.can_jump = true;
        }

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
  auto& ball_ball = registry.get<BallComponent>(ball);

  auto& player_physics = registry.get<PhysicsComponent>(object.entity);
  unsigned int player_team = TEAM_RED;
  if (registry.has<TeamComponent>(object.entity)) {
    player_team = registry.get<TeamComponent>(object.entity).team;
  }

  ball_hitbox.center += object.move_vector;
  ball_physics.is_airborne = true;

  unsigned int faker_team = ball_ball.faker_team;

  float ball_speed = glm::length(ball_physics.velocity);
  float player_speed = glm::length(player_physics.velocity);
  if (ball_speed < player_speed) {
    ball_physics.velocity =
        object.normal * (glm::dot(player_physics.velocity, object.normal));

  } else {
    BallCollision(&ball_physics, object.normal);  // player_physics.velocity);
    if (ball_ball.is_super_striked) {
      player_physics.velocity = ball_physics.velocity * 0.5f;
      player_physics.is_airborne = true;
    }
  }

  if (ball_physics.velocity.y >= 0 ||
      ball_physics.velocity.y < -1.f / 128 * 9.82 * 6.f) {
    // save game event
    if (registry.has<IDComponent>(ball)) {
      GameEvent nudge_event;
      nudge_event.type = GameEvent::NUDGE;
      nudge_event.nudge.ball_id = registry.get<IDComponent>(ball).id;
      dispatcher.trigger(nudge_event);
    }
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

      if (data.move_vector.y < 0.f) {
        auto& player_c = registry.get<PlayerComponent>(object.entity);
        player_c.can_jump = true;

        // save game event
        if (registry.has<IDComponent>(object.entity) &&
            player_physics.velocity.y < -1.f) {
          GameEvent land_event;
          land_event.type = GameEvent::LAND;
          land_event.land.player_id =
              registry.get<IDComponent>(object.entity).id;
          dispatcher.trigger(land_event);
        }

        player_physics.velocity.y = 0.f;
      }

      BallCollision(&ball_physics, object.normal);
    }
  }

  if (faker_team != player_team && !ball_ball.is_real) {
    EventInfo e;
    e.event = Event::DESTROY_ENTITY;
    e.entity = ball;
    e.e_id = registry.get<IDComponent>(ball).id;
    dispatcher.enqueue<EventInfo>(e);
  }

  return;
}

void BallArenaCollision(entt::registry& registry, const CollisionObject& object,
                        entt::entity ball) {
  auto& ball_physics = registry.get<PhysicsComponent>(ball);
  auto& ball_hitbox = registry.get<physics::Sphere>(ball);
  auto& ball_c = registry.get<BallComponent>(ball);

  ball_hitbox.center += object.move_vector;

  bool bounced = false;

  float dot_val = glm::dot(object.normal, ball_physics.velocity);
  if (dot_val < 0.f) {
    ball_physics.velocity =
        ball_physics.velocity - object.normal * dot_val * 0.7f * 2.f;
    bounced = true;

    if (object.normal.y > 0.f) {
      ball_physics.is_airborne = true;
    }
  }

  if (object.normal.y == 1.f) {
    // Ball hits the ground
    if (ball_physics.velocity.y < 0.8f) {
      ball_physics.velocity.y = 0.f;
      ball_physics.is_airborne = false;
      ball_c.rotation = glm::quat();
      bounced = false;
    }
  }

  if (bounced) {
    // save game event
    if (registry.has<IDComponent>(ball)) {
      GameEvent bounce_event;
      bounce_event.type = GameEvent::BOUNCE;
      bounce_event.bounce.ball_id = registry.get<IDComponent>(ball).id;
      dispatcher.trigger(bounce_event);
    }
  }

  // Rotate ball
  glm::vec3 nn = glm::normalize(object.normal);
  glm::vec3 dir =
      ball_physics.velocity - glm::dot(ball_physics.velocity, nn) * nn;

  if (glm::length(dir) == 0) return;

  if (ball_c.is_super_striked) {
    ball_physics.velocity *= 0.3f;
    ball_c.is_super_striked = false;
  }

  glm::vec3 rotate = glm::normalize(glm::cross(nn, dir));
  float amount = glm::length(dir);
  amount *= 0.7;
  ball_c.rotation = glm::quat(0, rotate * amount);

  return;
}

void BallWallCollision(entt::registry& registry, const CollisionObject& object,
                       entt::entity ball) {
  auto& ball_physics = registry.get<PhysicsComponent>(ball);
  auto& ball_hitbox = registry.get<physics::Sphere>(ball);
  auto& ball_c = registry.get<BallComponent>(ball);

  ball_hitbox.center += object.move_vector;

  bool bounced = false;

  float dot_val = glm::dot(object.normal, ball_physics.velocity);
  if (dot_val < 0.f) {
    ball_physics.velocity =
        ball_physics.velocity - object.normal * dot_val * 0.8f * 2.f;
    bounced = true;
  }
  if (object.normal.y > 0.5f) {
    ball_physics.is_airborne = true;
  }

  if (object.normal.y == 1) {
    bounced = false;
  }

  if (bounced) {
    // save game event
    if (registry.has<IDComponent>(ball)) {
      GameEvent bounce_event;
      bounce_event.type = GameEvent::BOUNCE;
      bounce_event.bounce.ball_id = registry.get<IDComponent>(ball).id;
      dispatcher.trigger(bounce_event);
    }
  }

  // Rotate ball
  glm::vec3 nn = glm::normalize(object.normal);
  glm::vec3 dir =
      ball_physics.velocity - glm::dot(ball_physics.velocity, nn) * nn;

  if (glm::length(dir) == 0) return;

  if (ball_c.is_super_striked) {
    ball_physics.velocity *= 0.3f;
    ball_c.is_super_striked = false;
  }

  glm::vec3 rotate = glm::normalize(glm::cross(nn, dir));
  float amount = glm::length(dir);
  amount *= 0.7;
  ball_c.rotation = glm::quat(0, rotate * amount);

  return;
}

void PlayerArenaCollision(entt::registry& registry) {
  auto view_player = registry.view<physics::OBB, PhysicsComponent>();
  auto view_mesh_arena =
      registry.view<physics::MeshHitbox, FailSafeArenaComponent>();
  for (auto player : view_player) {
    auto& player_hitbox = view_player.get<physics::OBB>(player);
    auto& physics_c = view_player.get<PhysicsComponent>(player);
    FailSafeArenaComponent arena_hitbox2;
    for (auto arena : view_mesh_arena) {
      auto& arena_hitbox = view_mesh_arena.get<physics::MeshHitbox>(arena);
      arena_hitbox2 = view_mesh_arena.get<FailSafeArenaComponent>(arena);

      physics::IntersectData data =
          Intersect(arena_hitbox, player_hitbox, -physics_c.velocity);
      if (data.collision == false)
        data = Intersect(arena_hitbox2.arena, player_hitbox);
      if (data.collision) {
        player_hitbox.center += data.move_vector;
        if (data.normal.y > 0.25 && physics_c.velocity.y < 0) {
          auto& player_c = registry.get<PlayerComponent>(player);
          // save game event
          if (registry.has<IDComponent>(player) &&
              physics_c.velocity.y < -1.0f) {
            GameEvent land_event;
            land_event.type = GameEvent::LAND;
            land_event.land.player_id = registry.get<IDComponent>(player).id;
            dispatcher.trigger(land_event);
          }
          physics_c.velocity.y = 0.f;
          player_c.can_jump = true;
        } else if (data.move_vector.y < 0.0f) {
          physics_c.velocity.y = 0.f;
        }
      }
    }
    if (player_hitbox.center.x > arena_hitbox2.arena.xmax - 0.9f) {
      player_hitbox.center.x = arena_hitbox2.arena.xmax - 0.9f;
    } else if (player_hitbox.center.x < arena_hitbox2.arena.xmin + 0.9f) {
      player_hitbox.center.x = arena_hitbox2.arena.xmin + 0.9f;
    }

    if (player_hitbox.center.y - player_hitbox.extents[1] <=
            arena_hitbox2.arena.ymin &&
        physics_c.velocity.y < 0) {
      physics_c.velocity.y = 0.0f;
      player_hitbox.center.y =
          arena_hitbox2.arena.ymin + player_hitbox.extents[1];
    }
  }

  return;
}

void PlayerWallCollision(entt::registry& registry) {
  auto view_player =
      registry.view<physics::OBB, PhysicsComponent, PlayerComponent>();
  auto view_wall = registry.view<physics::OBB, WallComponent>();

  for (auto player : view_player) {
    auto& player_hitbox = view_player.get<physics::OBB>(player);

    for (auto wall : view_wall) {
      auto& wall_hitbox = registry.get<physics::OBB>(wall);

      physics::IntersectData data = Intersect(wall_hitbox, player_hitbox);
      if (data.collision) {
        auto& physics = view_player.get<PhysicsComponent>(player);
        player_hitbox.center -= data.move_vector;

        if (registry.has<IDComponent>(player) &&
            player_hitbox.center.y >
                wall_hitbox.center.y + wall_hitbox.extents[1]) {
          if (physics.velocity.y < -0.3f) {
            // save game event
            GameEvent land_event;
            land_event.type = GameEvent::LAND;
            land_event.land.player_id = registry.get<IDComponent>(player).id;
            dispatcher.trigger(land_event);
          }
          physics.velocity.y = 0.f;
          auto& player_c = view_player.get<PlayerComponent>(player);
          player_c.can_jump = true;
        }

        if (data.normal.y < -0.5f) {
          physics.velocity.y = 0.f;
        }
        // auto& health = registry.get<HealthComponent>(wall);
        // health.health -= 1;
      }
    }
  }
}

void BallBallCollision(entt::registry& registry) {
  auto view_ball =
      registry.view<BallComponent, physics::Sphere, PhysicsComponent>();

  for (auto it = view_ball.begin(); it != view_ball.end(); ++it) {
    bool collide = false;
    auto& hitbox1 = view_ball.get<physics::Sphere>(*it);
    auto& physics1 = view_ball.get<PhysicsComponent>(*it);
    auto& ball1 = view_ball.get<BallComponent>(*it);

    for (auto it3 = view_ball.begin(); it3 != it; ++it3) {
      auto& hitbox2 = view_ball.get<physics::Sphere>(*it3);

      physics::IntersectData data = Intersect(hitbox1, hitbox2);
      if (data.collision) {
        collide = true;
      }
    }

    for (auto it2 = std::next(it, 1); it2 != view_ball.end(); ++it2) {
      auto& hitbox2 = view_ball.get<physics::Sphere>(*it2);
      auto& physics2 = view_ball.get<PhysicsComponent>(*it2);
      auto& ball2 = view_ball.get<BallComponent>(*it2);

      physics::IntersectData data = Intersect(hitbox1, hitbox2);
      if (data.collision) {
        collide = true;

        if (ball1.can_ball_collide && ball2.can_ball_collide) {
          float dot_val1 = glm::dot(data.normal, physics1.velocity);
          float dot_val2 = glm::dot(data.normal, physics2.velocity);

          physics1.velocity +=
              data.normal * dot_val2 * 0.8f - data.normal * dot_val1 * 0.8f;
          physics2.velocity +=
              data.normal * dot_val1 * 0.8f - data.normal * dot_val2 * 0.8f;

          float vel1 = glm::length(physics1.velocity);
          float vel2 = glm::length(physics2.velocity);
          float total = vel1 + vel2 + 0.0001f;
          hitbox1.center += data.move_vector * vel1 / total;
          hitbox2.center -= data.move_vector * vel2 / total;

          physics1.is_airborne = true;
          physics2.is_airborne = true;
        } else {
          if (data.move_vector == glm::vec3(0.f))
            data.normal = glm::vec3(1.f, 0.f, 0.f);
          hitbox1.center += data.normal * 0.01f;
          hitbox2.center -= data.normal * 0.01f;
        }
      }
    }

    if (collide == false) {
      ball1.can_ball_collide = true;
    }
  }
}

void PlayerPlayerCollision(entt::registry& registry) {
  auto view_player =
      registry.view<physics::OBB, PhysicsComponent, PlayerComponent>();

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
          } else {
            dot_val2 = 0.f;
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
          auto& player_c = view_player.get<PlayerComponent>(player2);
          player_c.can_jump = true;

          // save game event
          if (registry.has<IDComponent>(player2) &&
              physics_c2.velocity.y < -1.f) {
            GameEvent land_event;
            land_event.type = GameEvent::LAND;
            land_event.land.player_id = registry.get<IDComponent>(player2).id;
            dispatcher.trigger(land_event);
          }
        } else if (glm::dot(glm::vec3(0.f, 1.f, 0.f), data.normal) > 0.f) {
          physics_c1.velocity.y = 0.f;
          auto& player_c = view_player.get<PlayerComponent>(player1);
          player_c.can_jump = true;

          // save game event
          if (registry.has<IDComponent>(player1) &&
              physics_c1.velocity.y < -1.f) {
            GameEvent land_event;
            land_event.type = GameEvent::LAND;
            land_event.land.player_id = registry.get<IDComponent>(player1).id;
            dispatcher.trigger(land_event);
          }
        }
      }
    }
  }
}

void PlayerProjectileCollision(entt::registry& registry) {
  auto view_player =
      registry
          .view<physics::OBB, PhysicsComponent, IDComponent, PlayerComponent>();
  auto view_projectile = registry.view<physics::Sphere, ProjectileComponent>();

  for (auto player : view_player) {
    auto& player_hitbox = registry.get<physics::OBB>(player);
    auto& player_c = registry.get<PlayerComponent>(player);
    auto& player_id = registry.get<IDComponent>(player);
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& id = view_projectile.get<ProjectileComponent>(projectile);

      physics::IntersectData data = Intersect(proj_hitbox, player_hitbox);
      if (data.collision && id.creator != player_c.client_id) {
        switch (id.projectile_id) {
          case ProjectileID::CANNON_BALL: {
            // registry.destroy(projectile);
            DestroyEntity(registry, projectile);
            break;
          }
          case ProjectileID::FORCE_PUSH_OBJECT: {
            ApplyForcePush(registry, proj_hitbox.center);

            // Save game event
            GameEvent force_push_impact_event;
            force_push_impact_event.type = GameEvent::FORCE_PUSH_IMPACT;
            force_push_impact_event.force_push_impact.projectile_id =
                player_id.id;
            dispatcher.trigger(force_push_impact_event);
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
  auto view_projectile =
      registry.view<physics::Sphere, ProjectileComponent, IDComponent>();
  auto& ball_hitbox = registry.get<physics::Sphere>(ball);
  auto& ball_c = registry.get<BallComponent>(ball);
  auto& ball_physics = registry.get<PhysicsComponent>(ball);

  for (auto projectile : view_projectile) {
    auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
    auto& proj_c = view_projectile.get<ProjectileComponent>(projectile);
    auto& id_c = view_projectile.get<IDComponent>(projectile);

    unsigned int creator_team = TEAM_NONE;

    auto view_players =
        registry.view<PlayerComponent, IDComponent, TeamComponent>();
    for (auto player : view_players) {
      long id = registry.get<PlayerComponent>(player).client_id;
      if (id == proj_c.creator) {
        creator_team = registry.get<TeamComponent>(player).team;
      }
    }

    physics::IntersectData data = Intersect(ball_hitbox, proj_hitbox);
    if (data.collision) {
      switch (proj_c.projectile_id) {
        case ProjectileID::CANNON_BALL: {
          glm::vec3 dir = normalize(ball_hitbox.center - proj_hitbox.center);
          ball_physics.velocity = dir * 20.0f;
          ball_physics.is_airborne = true;
          ball_c.last_touch = proj_c.creator;
          // registry.destroy(projectile);

          if (!ball_c.is_real && ball_c.faker_team != creator_team &&
              creator_team != TEAM_NONE) {
            /*EventInfo e;
            e.event = Event::DESTROY_ENTITY;
            e.entity = ball;
            e.e_id = registry.get<IDComponent>(ball).id;
            dispatcher.enqueue<EventInfo>(e);*/
            ball_c.should_be_destroyed = true;
          }
          DestroyEntity(registry, projectile);
          break;
        }
        case ProjectileID::FORCE_PUSH_OBJECT: {
          ApplyForcePush(registry, proj_hitbox.center);

          // Save game event
          GameEvent force_push_impact_event;
          force_push_impact_event.type = GameEvent::FORCE_PUSH_IMPACT;
          force_push_impact_event.force_push_impact.projectile_id = id_c.id;
          dispatcher.trigger(force_push_impact_event);

          DestroyEntity(registry, projectile);
          break;
        }
      }
    }
  }

  return;
}

void ProjectileArenaCollision(entt::registry& registry) {
  auto view_arena =
      registry.view<FailSafeArenaComponent, physics::MeshHitbox>();
  auto view_projectile =
      registry.view<physics::Sphere, ProjectileComponent, IDComponent>();

  for (auto arena : view_arena) {
    auto& arena_hitbox = view_arena.get<physics::MeshHitbox>(arena);
    auto& fail_safe_arena_hitbox =
        view_arena.get<FailSafeArenaComponent>(arena);
    for (auto projectile : view_projectile) {
      auto& proj_hitbox = view_projectile.get<physics::Sphere>(projectile);
      auto& proj_id = view_projectile.get<ProjectileComponent>(projectile);
      auto& id_c = view_projectile.get<IDComponent>(projectile);

      physics::IntersectData data = Intersect(arena_hitbox, proj_hitbox);
      if (data.collision == false) {
        data = Intersect(fail_safe_arena_hitbox.arena, proj_hitbox);
      }

      if (data.collision) {
        switch (proj_id.projectile_id) {
          case ProjectileID::CANNON_BALL: {
            // registry.destroy(projectile);
            DestroyEntity(registry, projectile);
            break;
          }
          case ProjectileID::FORCE_PUSH_OBJECT: {
            ApplyForcePush(registry, proj_hitbox.center);

            // Save game event
            GameEvent force_push_impact_event;
            force_push_impact_event.type = GameEvent::FORCE_PUSH_IMPACT;
            force_push_impact_event.force_push_impact.projectile_id = id_c.id;
            dispatcher.trigger(force_push_impact_event);

            DestroyEntity(registry, projectile);
            break;
          }
          case ProjectileID::MISSILE_OBJECT: {
            // ApplyForcePush(registry, proj_hitbox.center);
            // Save game event
            GameEvent missile_impact_event;
            missile_impact_event.type = GameEvent::MISSILE_IMPACT;
            missile_impact_event.missile_impact.projectile_id = id_c.id;
            dispatcher.trigger(missile_impact_event);

            DestroyEntity(registry, projectile);
            break;
          }
          case ProjectileID::TELEPORT_PROJECTILE: {
            // Teleport to collision site
            TeleportToCollision(
                registry,
                proj_hitbox.center + data.normal * 1.0f + data.move_vector,
                proj_id.creator);
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

        int num_abilities =
            static_cast<typename std::underlying_type<AbilityID>::type>(
                AbilityID::NUM_OF_ABILITY_IDS);
        AbilityID pickup_ability =
            static_cast<AbilityID>(rand() % (num_abilities - 1) + 1);

        registry.assign<PickUpEvent>(
            entity, registry.get<IDComponent>(pick_up).id,
            view_player.get<PlayerComponent>(player).client_id, pickup_ability);

        auto& ability_c = view_player.get<AbilityComponent>(player);
        ability_c.secondary_ability = pickup_ability;

        registry.destroy(pick_up);
      }
    }
  }

  return;
}

void MinePlayerCollision(entt::registry& registry) {
  auto mine_view = registry.view<MineComponent, TransformComponent, IDComponent>();
  auto player_view =
      registry.view<PlayerComponent, TransformComponent, PhysicsComponent,
                    TeamComponent, IDComponent>();
 
  for (auto mine : mine_view) {
    auto& mine_c = mine_view.get<MineComponent>(mine);
    auto& mine_tc = mine_view.get<TransformComponent>(mine);
    auto& mine_idc = mine_view.get<IDComponent>(mine);

    for (auto player : player_view) {
      auto& p_tc = player_view.get<TransformComponent>(player);
      auto& p_pc = player_view.get<PhysicsComponent>(player);
      auto& p_teamc = player_view.get<TeamComponent>(player);
      auto& p_idc = player_view.get<IDComponent>(player);

      // TODO: Only trigger from collision with player from opposing team
      if (glm::length(mine_tc.position - p_tc.position) <=
              GlobalSettings::Access()->ValueOf(
                  "ABILITY_MINE_TRIGGER_RADIUS") &&
          mine_c.owner_team != p_teamc.team) {

        ApplyMineStun(p_pc);

        // Save game event
        GameEvent mine_trigger_event;
        mine_trigger_event.type = GameEvent::MINE_TRIGGER;
        mine_trigger_event.mine_trigger.player_id = p_idc.id;
        dispatcher.trigger(mine_trigger_event);

        DestroyEntity(registry, mine);
      }
    }
  }
}

void BallCollision(PhysicsComponent* ball, glm::vec3 normal) {
  float dot_val = glm::dot(ball->velocity, normal);
  if (dot_val < 0.f)
    ball->velocity = ball->velocity - normal * dot_val * 0.8f * 2.f;
}

void SetJumpToFalse(entt::registry& reg) {
  auto player_view = reg.view<PlayerComponent>();

  for (auto player : player_view) {
    auto& player_c = player_view.get(player);
    player_c.can_jump = false;
  }
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

void ApplyForcePushOnEntity(glm::vec3 explosion_pos, glm::vec3 entity_pos,
                            PhysicsComponent& physics_c) {
  physics::Sphere force_push;
  force_push.center = explosion_pos;
  force_push.radius =
      GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_RADIUS");
  glm::vec3 dir = entity_pos - force_push.center;
  float length = glm::length(dir);
  std::cout << "FORCEPUSH Length: " << length;
  if (length < force_push.radius) {
    physics_c.is_airborne = true;
    float force =
        GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_STRENGTH");
    dir = glm::normalize(dir);
    physics_c.velocity =
        dir * force * (force_push.radius - length) / force_push.radius;
    std::cout << " Velocity: " << physics_c.velocity;
  }
  std::cout << std::endl;
}

void ApplyMineStun(PhysicsComponent& physics_c) {
  physics_c.velocity += glm::vec3(0.f, 5.f, 0.f);
}

void TeleportToCollision(entt::registry& registry, glm::vec3 hit_pos,
                         long player_id) {
  auto player_view =
      registry.view<PlayerComponent, TransformComponent, PhysicsComponent,
                    physics::OBB, IDComponent>();
  for (auto player : player_view) {
    auto& player_c = player_view.get<PlayerComponent>(player);
    auto& phys_c = player_view.get<PhysicsComponent>(player);
    auto& obb_c = player_view.get<physics::OBB>(player);
    auto& id_c = player_view.get<IDComponent>(player);

    if (player_c.client_id == player_id) {
      phys_c.is_airborne = true;
      obb_c.center = hit_pos;

      // Save game event
      GameEvent teleport_impact_event;
      teleport_impact_event.type = GameEvent::TELEPORT_IMPACT;
      teleport_impact_event.teleport_impact.player_id = id_c.id;
      teleport_impact_event.teleport_impact.hit_pos = hit_pos;
      dispatcher.trigger(teleport_impact_event);

      break;
    }
  }
}

void EndHomingBall(entt::registry& registry, entt::entity& in_ball) {
  BallComponent& ball_c = registry.get<BallComponent>(in_ball);
  ball_c.is_homing = false;
  // Save game event
  if (registry.has<IDComponent>(in_ball)) {
    IDComponent& ball_id_c = registry.get<IDComponent>(in_ball);
    GameEvent homing_ball_end_event;
    homing_ball_end_event.type = GameEvent::HOMING_BALL_END;
    homing_ball_end_event.homing_ball_end.ball_id = ball_id_c.id;
    dispatcher.trigger(homing_ball_end_event);
  }
}

void DestroyEntity(entt::registry& registry, entt::entity entity) {
  EventInfo info;
  if (registry.has<IDComponent>(entity) == false) return;
  auto id = registry.get<IDComponent>(entity);
  info.event = Event::DESTROY_ENTITY;
  info.e_id = id.id;
  info.entity = entity;
  dispatcher.enqueue<EventInfo>(info);
  // registry.destroy(entity);
}
#endif  // COLLISION_SYSTEM_HPP_