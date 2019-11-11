#ifndef ABILITY_CONTROLLER_SYSTEM_HPP_
#define ABILITY_CONTROLLER_SYSTEM_HPP_

#include <entt.hpp>
//#include <position.h>
#include <boundingboxes.hpp>
#include "ecs/components.hpp"
#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"
#include "util/event.hpp"

#include "util/event.hpp"
#include "util/global_settings.hpp"
#include "util/timer.hpp"

#include <physics.hpp>
#include "ecs/systems/missile_system.hpp"

namespace ability_controller {
Timer gravity_timer;
Timer blackout_timer;
bool gravity_used = false;
bool blackout_used = false;
// TODO: Make unique for each player

bool TriggerAbility(entt::registry& registry, AbilityID in_a_id,
                    PlayerID player_id, entt::entity caster);
void CreateMissileEntity(entt::registry& registry, PlayerID id);
bool DoSuperStrike(entt::registry& registry);
entt::entity CreateCannonBallEntity(entt::registry& registry, PlayerID id);
void DoSwitchGoals(entt::registry& registry);
entt::entity CreateForcePushEntity(entt::registry& registry, PlayerID id);
void GravityChange();
void DoTeleport(entt::registry& registry, PlayerID id);
bool DoHomingBall(entt::registry& registry, PlayerID id);
void CreateFakeBalls(entt::registry& registry, EntityID id);
bool BuildWall(entt::registry& registry, PlayerID id);
bool DoInvisibility(entt::registry& registry, PlayerID id);
bool DoBlackout(entt::registry& registry);

std::unordered_map<AbilityID, float> ability_cooldowns;

void Update(entt::registry& registry, float dt) {
  auto view_players = registry.view<PlayerComponent, TransformComponent,
                                    AbilityComponent, IDComponent>();
  for (auto player : view_players) {
    auto& ability_component = registry.get<AbilityComponent>(player);
    auto& transform_component = registry.get<TransformComponent>(player);
    auto& player_component = registry.get<PlayerComponent>(player);
    auto& id_component = registry.get<IDComponent>(player);

    // Loop over each entity with a PlayerComponent
    // and Ability Component

    // Update cooldowns
    ability_component.cooldown_remaining -= dt;
    ability_component.shoot_cooldown -= dt;
    if (ability_component.cooldown_remaining < 0.0f) {
      ability_component.cooldown_remaining = 0.0f;
    }
    if (ability_component.shoot_cooldown < 0.0f) {
      ability_component.shoot_cooldown = 0.0f;
    }
    // Update invisibility duration
    if (player_component.invisible) {
      player_component.invisibility_remaining -= dt;
    }

    // First check if primary ability is being used
    // AND ability is not on cooldown
    if (ability_component.use_primary &&
        !(ability_component.cooldown_remaining > 0.0f)) {
      // Trigger the ability
      if (TriggerAbility(registry, ability_component.primary_ability,
                         player_component.client_id, player)) {
        // If ability triggered successfully set the
        // AbilityComponent's cooldown to be on max capacity
        ability_component.cooldown_remaining =
            ability_cooldowns[ability_component.primary_ability];
        GameEvent primary_used_event;
        primary_used_event.type = GameEvent::PRIMARY_USED;
        primary_used_event.primary_used.player_id = id_component.id;
        primary_used_event.primary_used.cd =
            ability_component.cooldown_remaining;
        dispatcher.trigger(primary_used_event);
      }
    }
    // When finished set primary ability to not activated
    ability_component.use_primary = false;

    // Then check if secondary ability is being used
    if (ability_component.use_secondary) {
      // Trigger the ability
      if (TriggerAbility(registry, ability_component.secondary_ability,
                         player_component.client_id, player)) {
        // If ability triggered successfully, remove the
        // slotted secondary ability
        ability_component.secondary_ability = AbilityID::NULL_ABILITY;

        GameEvent secondary_used_event;
        secondary_used_event.type = GameEvent::SECONDARY_USED;
        secondary_used_event.secondary_used.player_id = id_component.id;
        dispatcher.trigger(secondary_used_event);
        // TODO: send that ability is used
      }
    }
    // When finished set secondary ability to not activated
    ability_component.use_secondary = false;

    // Check if player is invisible
    if (player_component.invisible &&
        player_component.invisibility_remaining <= 0.0f) {
      player_component.invisible = false;
      // Save game event
      GameEvent invisibility_end_event;
      invisibility_end_event.type = GameEvent::INVISIBILITY_END;
      invisibility_end_event.invisibility_end.player_id = id_component.id;
      dispatcher.trigger(invisibility_end_event);
    }

    // Check if the player should shoot
    if (ability_component.shoot && ability_component.shoot_cooldown <= 0.0f) {
      entt::entity entity =
          CreateCannonBallEntity(registry, player_component.client_id);
      ability_component.shoot_cooldown = 1.0f;
      EventInfo e;
      e.event = Event::CREATE_CANNONBALL;
      e.entity = entity;
      dispatcher.enqueue<EventInfo>(e);
    }
    ability_component.shoot = false;
  };

  if (gravity_used &&
      gravity_timer.Elapsed() >=
          GlobalSettings::Access()->ValueOf("ABILITY_GRAVITY_DURATION")) {
    gravity_used = false;
    physics::SetGravity(GlobalSettings::Access()->ValueOf("PHYSICS_GRAVITY"));
  }
  if (blackout_used &&
      blackout_timer.Elapsed() >=
          GlobalSettings::Access()->ValueOf("ABILITY_BLACKOUT_DURATION")) {
    blackout_used = false;
    // Save game event, turn on lights on client
    GameEvent event;
    event.type = GameEvent::BLACKOUT_END;
    dispatcher.trigger<GameEvent>(event);
  }
}

bool TriggerAbility(entt::registry& registry, AbilityID in_a_id,
                    PlayerID player_id, entt::entity caster) {
  switch (in_a_id) {
    case AbilityID::NULL_ABILITY:
      return false;
      break;
    case AbilityID::BUILD_WALL:
      return BuildWall(registry, player_id);
      break;
    case AbilityID::FAKE_BALL:
      CreateFakeBalls(registry, player_id);
      return true;
      break;
    case AbilityID::FORCE_PUSH: {
      entt::entity entity = CreateForcePushEntity(registry, player_id);
      EventInfo e;
      e.event = Event::CREATE_FORCE_PUSH;
      e.entity = entity;
      dispatcher.enqueue<EventInfo>(e);
      return true;
      break;
    }
    case AbilityID::GRAVITY_CHANGE:
      GravityChange();
      return true;
      break;
    case AbilityID::HOMING_BALL:
      return DoHomingBall(registry, player_id);
      break;
    case AbilityID::INVISIBILITY:
      return DoInvisibility(registry, player_id);
      break;
    case AbilityID::MISSILE:
      CreateMissileEntity(registry, player_id);
      return true;
      break;
    case AbilityID::SUPER_STRIKE:
      return DoSuperStrike(registry);
      break;
    case AbilityID::SWITCH_GOALS:
      DoSwitchGoals(registry);
      return true;
      break;
    case AbilityID::TELEPORT:
      DoTeleport(registry, player_id);
      return true;
      break;
    case AbilityID::BLACKOUT:
      return DoBlackout(registry);
      break;
    default:
      return false;
      break;
  }
}

void CreateMissileEntity(entt::registry& registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);

    if (pc.client_id == id) {
      float speed = 20.f;
      auto missile = registry.create();

      registry.assign<MissileComponent>(missile, speed, pc.target,
                                        pc.client_id);
      registry.assign<TransformComponent>(
          missile, tc.position + cc.GetLookDir(), cc.orientation);
      registry.assign<PhysicsComponent>(missile, cc.GetLookDir(),
                                        glm::vec3(0.f), true);
      registry.assign<physics::Sphere>(
          missile, glm::vec3(tc.position + cc.offset), 0.5f);  // center?
      registry.assign<ProjectileComponent>(
          missile, ProjectileID::MISSILE_OBJECT, pc.client_id);

      EventInfo e;
      e.event = Event::CREATE_MISSILE;
      e.entity = missile;
      dispatcher.enqueue<EventInfo>(e);

      break;
    }
  }
}

bool DoSuperStrike(entt::registry& registry) {
  // NTS: The logic of this function assumes that there is only one
  // entity with a PlayerComponent and that is the entity representing
  // the player on this client

  // Get the player and some other useful components
  auto player_view =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();

  // Now get all entities with a BallComponent
  auto ball_view =
      registry.view<BallComponent, PhysicsComponent, TransformComponent>();

  // Loop through all player entities (should only be one)
  for (auto player_entity : player_view) {
    CameraComponent& camera_c = player_view.get<CameraComponent>(player_entity);
    PlayerComponent& player_c = player_view.get<PlayerComponent>(player_entity);
    TransformComponent& trans_c_player =
        player_view.get<TransformComponent>(player_entity);

    // Loop through all ball entities
    for (auto ball_entity : ball_view) {
      // BallComponent &ball_c = ball_view.get<BallComponent>(ball_entity);
      PhysicsComponent& physics_c_ball =
          ball_view.get<PhysicsComponent>(ball_entity);
      TransformComponent& trans_c_ball =
          ball_view.get<TransformComponent>(ball_entity);
      BallComponent& ball_ball_c = ball_view.get<BallComponent>(ball_entity);

      // Calculate the vector from the player to the ball
      glm::vec3 player_ball_vec =
          trans_c_ball.position - trans_c_player.position;

      // Normalize the vector to get a directional vector
      glm::vec3 player_ball_dir = glm::normalize(player_ball_vec);

      // Get the direction the player is looking
      glm::vec3 player_look_dir = camera_c.GetLookDir();

      // Calculate the distance from player to ball
      float dist = length(player_ball_vec);

      // Calculate the dot-product between the direction the player
      // is looking and the direction the ball is in
      float dot = glm::dot(player_look_dir, player_ball_dir);

      // IF the distance is less than the player's reach
      // AND the ball lies within the player's field of view
      if (dist < player_c.kick_reach && dot > player_c.kick_fov) {
        // Calculate the direction the force should be applied in
        glm::vec3 kick_dir =
            camera_c.GetLookDir() + glm::vec3(0, player_c.kick_pitch, 0);

        // Apply the force of the kick to the ball's velocity
        physics_c_ball.velocity += kick_dir * GlobalSettings::Access()->ValueOf(
                                                  "ABILITY_SUPER_STRIKE_FORCE");
        physics_c_ball.is_airborne = true;
        ball_ball_c.is_super_striked = true;

        // Save game event
        if (registry.has<IDComponent>(player_entity)) {
          GameEvent super_kick_event;
          super_kick_event.type = GameEvent::SUPER_KICK;
          super_kick_event.super_kick.player_id =
              registry.get<IDComponent>(player_entity).id;
          dispatcher.trigger(super_kick_event);
        }
        return true;
      }
    }
  }
  return false;
}

entt::entity CreateCannonBallEntity(entt::registry& registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);

    if (pc.client_id == id) {
      float speed = pc.rocket_speed;
      auto cannonball = registry.create();
      registry.assign<PhysicsComponent>(cannonball,
                                        glm::vec3(cc.GetLookDir() * speed),
                                        glm::vec3(0.f), false, 0.0f);
      registry.assign<TransformComponent>(
          cannonball,
          glm::vec3(cc.GetLookDir() * 1.5f + tc.position + cc.offset),
          cc.orientation, glm::vec3(.3f, .3f, .3f));
      registry.assign<physics::Sphere>(cannonball,
                                       glm::vec3(tc.position + cc.offset), .3f);
      registry.assign<ProjectileComponent>(cannonball,
                                           ProjectileID::CANNON_BALL, id);
      // registry.assign<ModelComponent>(cannonball,
      //                                glob::GetModel("assets/Ball/Ball.fbx"));
      // registry.assign<LightComponent>(cannonball, glm::vec3(1, 0, 1), 3.f,
      // 0.f);
      return cannonball;
    }
  }
}

void DoSwitchGoals(entt::registry& registry) {
  auto view_goals = registry.view<GoalComponenet, TeamComponent>();
  GoalComponenet* first_goal_comp = nullptr;
  GoalComponenet* second_goal_comp = nullptr;
  bool got_first = false;
  for (auto goal : view_goals) {
    TeamComponent& goal_team_c = registry.get<TeamComponent>(goal);
    GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);

    if (goal_team_c.team == TEAM_RED) {
      goal_team_c.team = TEAM_BLUE;
      goal_goal_c.switched_this_tick = true;
    } else {
      goal_team_c.team = TEAM_RED;
      goal_goal_c.switched_this_tick = true;
    }
    if (!got_first) {
      first_goal_comp = &goal_goal_c;
      got_first = true;
    } else {
      second_goal_comp = &goal_goal_c;
    }
  }
  if (first_goal_comp != nullptr && second_goal_comp != nullptr) {
    unsigned int first_goals = first_goal_comp->goals;
    first_goal_comp->goals = second_goal_comp->goals;
    second_goal_comp->goals = first_goals;

    // Save game event
    GameEvent switch_goals_event;
    switch_goals_event.type = GameEvent::SWITCH_GOALS;
    dispatcher.trigger(switch_goals_event);
  }
}

entt::entity CreateForcePushEntity(entt::registry& registry, PlayerID id) {
  auto view_controller = registry.view<CameraComponent, PlayerComponent,
                                       TransformComponent, IDComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);
    IDComponent& idc = view_controller.get<IDComponent>(entity);

    if (pc.client_id == id) {
      float speed =
          GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_SPEED");
      auto force_object = registry.create();
      registry.assign<PhysicsComponent>(force_object, cc.GetLookDir() * speed,
                                        glm::vec3(0.f), true, 0.0f);
      registry.assign<TransformComponent>(
          force_object,
          glm::vec3(cc.GetLookDir() * 1.5f + tc.position + cc.offset),
          glm::vec3(0, 0, 0), glm::vec3(.5f, .5f, .5f));
      registry.assign<physics::Sphere>(force_object,
                                       glm::vec3(tc.position + cc.offset), .5f);
      registry.assign<ProjectileComponent>(force_object,
                                           ProjectileID::FORCE_PUSH_OBJECT, id);

      // Save game event
      GameEvent force_push_cast_event;
      force_push_cast_event.type = GameEvent::FORCE_PUSH;
      force_push_cast_event.force_push.player_id = idc.id;
      dispatcher.trigger(force_push_cast_event);

      return force_object;
    }
  }
}

void GravityChange() {
  physics::SetGravity(
      GlobalSettings::Access()->ValueOf("ABILITY_GRAVITY_CHANGE"));
  gravity_timer.Restart();
  gravity_used = true;

  GameEvent event;
  event.type = GameEvent::GRAVITY_DROP;
  dispatcher.trigger<GameEvent>(event);
}

void DoTeleport(entt::registry& registry, PlayerID id) {
  auto view_controller = registry.view<CameraComponent, PlayerComponent,
                                       TransformComponent, IDComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);
    IDComponent& idc = view_controller.get<IDComponent>(entity);

    if (pc.client_id == id) {
      float speed = 100.0f;
      auto teleport_projectile = registry.create();

      registry.assign<PhysicsComponent>(teleport_projectile,
                                        glm::vec3(cc.GetLookDir() * speed),
                                        glm::vec3(0.f), false, 0.0f);
      registry.assign<TransformComponent>(
          teleport_projectile,
          glm::vec3(cc.GetLookDir() * 1.5f + tc.position + cc.offset),
          glm::vec3(0, 0, 0), glm::vec3(.3f, .3f, .3f));
      registry.assign<physics::Sphere>(teleport_projectile,
                                       glm::vec3(tc.position + cc.offset), .3f);
      registry.assign<ProjectileComponent>(
          teleport_projectile, ProjectileID::TELEPORT_PROJECTILE, pc.client_id);

      EventInfo e;
      e.event = Event::CREATE_TELEPORT_PROJECTILE;
      e.entity = teleport_projectile;
      dispatcher.enqueue<EventInfo>(e);

      // Save game event
      GameEvent teleport_event;
      teleport_event.type = GameEvent::TELEPORT_CAST;
      teleport_event.teleport_cast.player_id = idc.id;
      dispatcher.trigger(teleport_event);

      break;
    }
  }
}

bool DoHomingBall(entt::registry& registry, PlayerID id) {
  auto view_players = registry.view<PlayerComponent, TransformComponent,
                                    CameraComponent, IDComponent>();
  for (auto player : view_players) {
    auto& p_p_c = registry.get<PlayerComponent>(player);
    auto& p_t_c = registry.get<TransformComponent>(player);
    auto& p_c_c = registry.get<CameraComponent>(player);
    auto& p_id_c = registry.get<IDComponent>(player);

    auto view_balls = registry.view<BallComponent, TransformComponent,
                                    PhysicsComponent, IDComponent>();
    for (auto ball : view_balls) {
      auto& b_b_c = registry.get<BallComponent>(ball);
      auto& b_t_c = registry.get<TransformComponent>(ball);
      auto& b_p_c = registry.get<PhysicsComponent>(ball);
      auto& b_id_c = registry.get<IDComponent>(ball);

      glm::vec3 player_ball_vec = b_t_c.position - p_t_c.position;

      glm::vec3 player_ball_dir = glm::normalize(player_ball_vec);
      glm::vec3 player_look_dir = p_c_c.GetLookDir();

      // Calculate the distance from player to ball
      float dist = length(player_ball_vec);

      // Calculate the dot-product between the direction the player
      // is looking and the direction the ball is in
      float dot = glm::dot(player_look_dir, player_ball_dir);

      // IF the distance is less than the player's reach
      // AND the ball lies within the player's field of view
      if (dist < p_p_c.kick_reach && dot > p_p_c.kick_fov && b_b_c.is_real) {
        // Save kick event
        GameEvent hit_event;
        hit_event.type = GameEvent::HIT;
        hit_event.hit.player_id = p_id_c.id;
        dispatcher.trigger(hit_event);

        glm::vec3 kick_dir =
            p_c_c.GetLookDir() + glm::vec3(0, p_p_c.kick_pitch, 0);

        b_b_c.homer_cid = p_p_c.client_id;
        b_b_c.is_homing = true;
        b_p_c.velocity += kick_dir * p_p_c.kick_force * 1.3f;
        b_p_c.is_airborne = true;
        missile_system::SetBallsAreHoming(true);

        // Save Homing event
        GameEvent homing_event;
        homing_event.type = GameEvent::HOMING_BALL;
        homing_event.homing_ball.ball_id = b_id_c.id;
        dispatcher.trigger(homing_event);

        return true;
      }
    }
  }

  return false;
}

bool BuildWall(entt::registry& registry, PlayerID id) {
  auto view_players = registry.view<PlayerComponent, TransformComponent,
                                    CameraComponent, IDComponent>();
  auto view_goals = registry.view<GoalComponenet, TransformComponent>();
  for (auto entity : view_players) {
    auto& player_c = view_players.get<PlayerComponent>(entity);

    if (player_c.client_id == id) {
      auto& trans_c = view_players.get<TransformComponent>(entity);
      auto& camera = view_players.get<CameraComponent>(entity);

      glm::vec3 position =
          camera.GetLookDir() * 4.5f + trans_c.position + camera.offset;
      position.y = -12.f;

      for (auto entity_goal : view_goals) {
        auto& goal_transform = view_goals.get<TransformComponent>(entity_goal);

        if (glm::distance(position, goal_transform.position) < 20.f) {
          return false;
        }
      }

      auto orientation = trans_c.rotation;
      // orientation.y = camera.GetLookDir().y;

      auto wall = registry.create();
      registry.assign<WallComponent>(wall);
      registry.assign<TimerComponent>(wall, 10.f);
      registry.assign<HealthComponent>(wall, 100);
      registry.assign<TransformComponent>(wall, position, orientation);
      auto& obb = registry.assign<physics::OBB>(wall);
      obb.extents[0] = 1.f;
      obb.extents[1] = 8.3f;
      obb.extents[2] = 5.f;

      EventInfo e;
      e.event = Event::BUILD_WALL;
      e.entity = wall;

      dispatcher.enqueue<EventInfo>(e);

      return true;
    }
  }

  return false;
}

bool DoInvisibility(entt::registry& registry, PlayerID id) {
  auto view_controller = registry.view<CameraComponent, PlayerComponent,
                                       TransformComponent, IDComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);
    IDComponent& idc = view_controller.get<IDComponent>(entity);

    if (pc.client_id == id) {
      pc.invisible = true;
      pc.invisibility_remaining =
          GlobalSettings::Access()->ValueOf("ABILITY_INVISIBILITY_DURATION");

      // Save game event
      GameEvent invisibility_event;
      invisibility_event.type = GameEvent::INVISIBILITY_CAST;
      invisibility_event.invisibility_cast.player_id = idc.id;
      dispatcher.trigger(invisibility_event);

      return true;
    }
  }
  return false;
}

bool DoBlackout(entt::registry& registry) {
  blackout_timer.Restart();
  blackout_used = true;

  // Save game event, turn off lights on client
  GameEvent event;
  event.type = GameEvent::BLACKOUT_CAST;
  dispatcher.trigger<GameEvent>(event);

  return true;
}

void CreateFakeBalls(entt::registry& registry, EntityID id) {
  int num_balls =
      (int)GlobalSettings::Access()->ValueOf("ABILITY_FAKEBALL_NUM_BALLS");

  unsigned int team = TEAM_RED;
  auto view_players =
      registry.view<PlayerComponent, TeamComponent, IDComponent>();

  // get the team that created fake balls
  for (auto player : view_players) {
    auto& p_t = registry.get<TeamComponent>(player);
    auto& p_p = registry.get<PlayerComponent>(player);

    if (p_p.client_id == id) {
      team = p_t.team;
      break;
    }
  }

  // get info about real ball
  glm::vec3 ball_pos, ball_vel;
  float friction = 1.0f;
  bool airborne = true;
  EntityID ball_id = 0;
  auto view_balls = registry.view<BallComponent, PhysicsComponent,
                                  TransformComponent, IDComponent>();
  for (auto ball : view_balls) {
    auto& ball_ball_c = registry.get<BallComponent>(ball);
    auto& ball_phys_c = registry.get<PhysicsComponent>(ball);
    auto& ball_trans_c = registry.get<TransformComponent>(ball);
    auto& ball_id_c = registry.get<IDComponent>(ball);

    if (ball_ball_c.is_real) {
      ball_pos = ball_trans_c.position;
      ball_vel = ball_phys_c.velocity;
      friction = ball_phys_c.friction;
      airborne = ball_phys_c.is_airborne;

      ball_id = ball_id_c.id;
      break;
    }
  }

  // create fake balls
  for (int i = 0; i < num_balls; i++) {
    glm::vec3 dir =
        glm::vec3((float)(rand() % 5) / 10.f, (float)(rand() % 5) / 10.f,
                  (float)(rand() % 5) / 10.f);
    dir = glm::normalize(dir);
    if (glm::length(ball_vel) > 0) {
      dir += ball_vel;
    }
    dir = glm::normalize(dir);
    glm::vec3 spawn_pos = ball_pos + dir;

    if (spawn_pos.y > ball_pos.y) {
      airborne = true;
    }

    auto fake_ball = registry.create();
    auto& fake_trans_c = registry.assign<TransformComponent>(fake_ball);
    fake_trans_c.position = spawn_pos;
    auto& fake_ball_c = registry.assign<BallComponent>(fake_ball);
    fake_ball_c.is_real = false;
    fake_ball_c.faker_team = team;
    fake_ball_c.is_airborne = airborne;
    auto& fake_phys_c = registry.assign<PhysicsComponent>(fake_ball);
    fake_phys_c.velocity = dir * glm::length(ball_vel);
    fake_phys_c.is_airborne = airborne;
    fake_phys_c.friction = friction;
    auto& fake_sphere_c = registry.assign<physics::Sphere>(fake_ball);
    fake_sphere_c.center = spawn_pos;
    fake_sphere_c.radius = 0.95f;
    EventInfo e;
    e.event = Event::CREATE_FAKE_BALL;
    e.entity = fake_ball;
    dispatcher.enqueue<EventInfo>(e);
  }
  GameEvent ge;
  ge.fake_ball_created.ball_id = ball_id;
  ge.type = GameEvent::FAKE_BALL_CREATED;
  dispatcher.trigger(ge);
}

};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
