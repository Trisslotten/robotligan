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
bool gravity_used = false;

bool TriggerAbility(entt::registry& registry, AbilityID in_a_id,
                    PlayerID player_id, entt::entity caster);
void CreateMissileEntity(entt::registry& registry, PlayerID id);
void DoSuperStrike(entt::registry& registry);
entt::entity CreateCannonBallEntity(entt::registry& registry, PlayerID id);
void DoSwitchGoals(entt::registry& registry);
entt::entity CreateForcePushEntity(entt::registry& registry, PlayerID id);
void GravityChange(entt::registry& registry);
void DoTeleport(entt::registry& registry, PlayerID id);
bool DoHomingBall(entt::registry& registry, PlayerID id);

void Update(entt::registry& registry, float dt) {
  auto view_players =
      registry.view<PlayerComponent, TransformComponent, AbilityComponent>();
  for (auto player : view_players) {
    auto& ability_component = registry.get<AbilityComponent>(player);
    auto& transform_component = registry.get<TransformComponent>(player);
    auto& player_component = registry.get<PlayerComponent>(player);

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

    // First check if primary ability is being used
    // AND ability is not on cooldown
    if (ability_component.use_primary &&
        !(ability_component.cooldown_remaining > 0.0f)) {
      // Trigger the ability
      if (TriggerAbility(registry, ability_component.primary_ability,
                         player_component.client_id, player)) {
        // If ability triggered successfully set the
        // AbilityComponent's cooldown to be on max capacity
        ability_component.cooldown_remaining = ability_component.cooldown_max;
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
        }
      }
      // Do not set use_secondary to false, it is done in serverstate to send over that the ability has been used
      // ability_component.use_secondary = false;

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
    physics::SetGravity(GlobalSettings::Access()->ValueOf("PHYSICS_GRAVITY"));
  }
}

bool TriggerAbility(entt::registry& registry, AbilityID in_a_id,
                    PlayerID player_id, entt::entity caster) {
  switch (in_a_id) {
    case AbilityID::NULL_ABILITY:
      return false;
      break;
    case AbilityID::BUILD_WALL:
      return true;
      break;
    case AbilityID::FAKE_BALL:
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
      GravityChange(registry);
      return true;
      break;
    case AbilityID::HOMING_BALL:
      return DoHomingBall(registry, player_id);
      break;
    case AbilityID::INVISIBILITY:
      return true;
      break;
    case AbilityID::MISSILE:
      CreateMissileEntity(registry, player_id);
      return true;
      break;
    case AbilityID::SUPER_STRIKE:
      DoSuperStrike(registry);
      return true;
      break;
    case AbilityID::SWITCH_GOALS:
      DoSwitchGoals(registry);
      return true;
      break;
    case AbilityID::TELEPORT:
      DoTeleport(registry, player_id);
      return true;
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

void DoSuperStrike(entt::registry& registry) {
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
      }
    }
  }
}

entt::entity CreateCannonBallEntity(entt::registry& registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);

    if (pc.client_id == id) {
      float speed = 20.0f;
      auto cannonball = registry.create();
      registry.assign<PhysicsComponent>(cannonball,
                                        glm::vec3(cc.GetLookDir() * speed),
                                        glm::vec3(0.f), false, 0.0f);
      registry.assign<TransformComponent>(
          cannonball,
          glm::vec3(cc.GetLookDir() * 1.5f + tc.position + cc.offset),
          glm::vec3(0, 0, 0), glm::vec3(.3f, .3f, .3f));
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
  }
}

entt::entity CreateForcePushEntity(entt::registry& registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);

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
      return force_object;
    }
  }
}

void GravityChange(entt::registry& registry) {
  physics::SetGravity(
      GlobalSettings::Access()->ValueOf("ABILITY_GRAVITY_CHANGE"));
  gravity_timer.Restart();
  gravity_used = true;
}

void DoTeleport(entt::registry& registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);

    if (pc.client_id == id) {
      float speed = 50.0f;
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

      break;
    }
  }
}

bool DoHomingBall(entt::registry& registry, PlayerID id) {
  auto view_players =
      registry.view<PlayerComponent, TransformComponent, CameraComponent>();
  for (auto player : view_players) {
    auto& p_p_c = registry.get<PlayerComponent>(player);
    auto& p_t_c = registry.get<TransformComponent>(player);
    auto& p_c_c = registry.get<CameraComponent>(player);

    auto view_balls =
        registry.view<BallComponent, TransformComponent, PhysicsComponent>();
    for (auto ball : view_balls) {
      auto& b_b_c = registry.get<BallComponent>(ball);
      auto& b_t_c = registry.get<TransformComponent>(ball);
      auto& b_p_c = registry.get<PhysicsComponent>(ball);

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
      if (dist < p_p_c.kick_reach && dot > p_p_c.kick_fov) {
        glm::vec3 kick_dir =
            p_c_c.GetLookDir() + glm::vec3(0, p_p_c.kick_pitch, 0);

        b_b_c.homer_cid = p_p_c.client_id;
        b_b_c.is_homing = true;
        b_p_c.velocity += kick_dir * p_p_c.kick_force * 1.3f;
        b_p_c.is_airborne = true;
        missile_system::SetBallsAreHoming(true);
		return true;
      }
    }
  }

  return false;
}
};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
