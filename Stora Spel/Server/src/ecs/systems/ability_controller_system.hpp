#ifndef ABILITY_CONTROLLER_SYSTEM_HPP_
#define ABILITY_CONTROLLER_SYSTEM_HPP_

#include <entt.hpp>
//#include <position.h>
#include <boundingboxes.hpp>
#include "ecs/components.hpp"
#include "shared/camera_component.hpp"
#include "util/event.hpp"

#include "util/event.hpp"
#include "util/global_settings.hpp"
#include "util/timer.hpp"

#include <physics.hpp>

namespace ability_controller {
Timer gravity_timer;
bool gravity_used = false;

bool TriggerAbility(entt::registry &registry, AbilityID in_a_id,
                    PlayerID player_id);
void CreateMissileEntity(entt::registry &registry);
void DoSuperStrike(entt::registry &registry);
entt::entity CreateCannonBallEntity(entt::registry &registry, PlayerID id);
void DoSwitchGoals(entt::registry &registry);
entt::entity CreateForcePushEntity(entt::registry &registry, PlayerID id);
void GravityChange(entt::registry &registry);

void Update(entt::registry &registry, float dt) {
  registry.view<PlayerComponent, AbilityComponent>().each(
      [&](PlayerComponent &player_component,
          AbilityComponent &ability_component) {
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
                             player_component.client_id)) {
            // If ability triggered successfully set the
            // AbilityComponent's cooldown to be on max capacity
            ability_component.cooldown_remaining =
                ability_component.cooldown_max;
          }
        }
        // When finished set primary ability to not activated
        ability_component.use_primary = false;

        // Then check if secondary ability is being used
        if (ability_component.use_secondary) {
          // Trigger the ability
          if (TriggerAbility(registry, ability_component.secondary_ability,
                             player_component.client_id)) {
            // If ability triggered successfully, remove the
            // slotted secondary ability
            ability_component.secondary_ability = AbilityID::NULL_ABILITY;
          }
        }
        // When finished set secondary ability to not activated
        ability_component.use_secondary = false;

        // Check if the player should shoot
        if (ability_component.shoot &&
            ability_component.shoot_cooldown <= 0.0f) {
          entt::entity entity =
              CreateCannonBallEntity(registry, player_component.client_id);
          ability_component.shoot_cooldown = 1.0f;
          EventInfo e;
          e.event = Event::CREATE_CANNONBALL;
          e.entity = entity;
          dispatcher.enqueue<EventInfo>(e);
        }
        ability_component.shoot = false;
      });

  if (gravity_used &&
      gravity_timer.Elapsed() >=
          GlobalSettings::Access()->ValueOf("ABILITY_GRAVITY_DURATION")) {
    physics::SetGravity(GlobalSettings::Access()->ValueOf("PHYSICS_GRAVITY"));
  }
}

bool TriggerAbility(entt::registry &registry, AbilityID in_a_id,
                    PlayerID player_id) {
  switch (in_a_id) {
    case AbilityID::NULL_ABILITY:
      break;
    case AbilityID::BUILD_WALL:
      break;
    case AbilityID::FAKE_BALL:
      break;
    case AbilityID::FORCE_PUSH: {
      entt::entity entity = CreateForcePushEntity(registry, player_id);
      EventInfo e;
      e.event = Event::CREATE_FORCE_PUSH;
      e.entity = entity;
      dispatcher.enqueue<EventInfo>(e);
      break;
    }
    case AbilityID::GRAVITY_CHANGE:
      GravityChange(registry);
      break;
    case AbilityID::HOMING_BALL:
      break;
    case AbilityID::INVISIBILITY:
      break;
    case AbilityID::MISSILE:
      CreateMissileEntity(registry);
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
      break;
    default:
      return false;
      break;
  }
}

void CreateMissileEntity(entt::registry &registry) {
  // Create new entity
  auto entity = registry.create();

  // WIP: Assign it the correct components here
}

void DoSuperStrike(entt::registry &registry) {
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
    CameraComponent &camera_c = player_view.get<CameraComponent>(player_entity);
    PlayerComponent &player_c = player_view.get<PlayerComponent>(player_entity);
    TransformComponent &trans_c_player =
        player_view.get<TransformComponent>(player_entity);

    // Loop through all ball entities
    for (auto ball_entity : ball_view) {
      // BallComponent &ball_c = ball_view.get<BallComponent>(ball_entity);
      PhysicsComponent &physics_c_ball =
          ball_view.get<PhysicsComponent>(ball_entity);
      TransformComponent &trans_c_ball =
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

entt::entity CreateCannonBallEntity(entt::registry &registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent &cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent &pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent &tc = view_controller.get<TransformComponent>(entity);

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

void DoSwitchGoals(entt::registry &registry) {
  auto view_goals = registry.view<GoalComponenet, TeamComponent>();
  GoalComponenet *first_goal_comp = nullptr;
  GoalComponenet *second_goal_comp = nullptr;
  bool got_first = false;
  for (auto goal : view_goals) {
    TeamComponent &goal_team_c = registry.get<TeamComponent>(goal);
    GoalComponenet &goal_goal_c = registry.get<GoalComponenet>(goal);

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

entt::entity CreateForcePushEntity(entt::registry &registry, PlayerID id) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();
  for (auto entity : view_controller) {
    CameraComponent &cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent &pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent &tc = view_controller.get<TransformComponent>(entity);

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

void GravityChange(entt::registry &registry) {
  physics::SetGravity(
      GlobalSettings::Access()->ValueOf("ABILITY_GRAVITY_CHANGE"));
  gravity_timer.Restart();
  gravity_used = true;
}

};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
