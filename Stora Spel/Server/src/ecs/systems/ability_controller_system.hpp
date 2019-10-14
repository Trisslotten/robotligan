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

namespace ability_controller {

bool TriggerAbility(entt::registry &registry, AbilityID in_a_id,
                    PlayerID player_id, entt::entity caster);
void CreateMissileEntity(entt::registry &registry, entt::entity caster);
void DoSuperStrike(entt::registry &registry);
entt::entity CreateCannonBallEntity(entt::registry &registry, PlayerID id);
void DoSwitchGoals(entt::registry &registry);
entt::entity CreateForcePushEntity(entt::registry &registry, PlayerID id);

void Update(entt::registry &registry, float dt) {
  auto view_players =
      registry.view<PlayerComponent, TransformComponent, AbilityComponent>();
  for (auto player : view_players) {
    auto &ability_component = registry.get<AbilityComponent>(player);
    auto &transform_component = registry.get<TransformComponent>(player);
    auto &player_component = registry.get<PlayerComponent>(player);

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
    // When finished set secondary ability to not activated
    ability_component.use_secondary = false;

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
  }
}

bool TriggerAbility(entt::registry &registry, AbilityID in_a_id,
                    PlayerID player_id, entt::entity caster) {
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
      break;
    case AbilityID::HOMING_BALL:
      break;
    case AbilityID::INVISIBILITY:
      break;
    case AbilityID::MISSILE:
      CreateMissileEntity(registry, caster);
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

void CreateMissileEntity(entt::registry &registry, entt::entity caster) {
  // Create new entity
  auto &player_c = registry.get<PlayerComponent>(caster);
  auto &trans_c = registry.get<TransformComponent>(caster);
  auto &cam_c = registry.get<CameraComponent>(caster);

  auto missile = registry.create();
  registry.assign<MissileComponent>(missile, 20.f, player_c.target, player_c.client_id);
  auto &missile_trans_c = registry.assign<TransformComponent>(missile);
  missile_trans_c.position = trans_c.position + cam_c.GetLookDir();
  missile_trans_c.rotation = cam_c.orientation;

  auto& missile_phys_c = registry.assign<PhysicsComponent>(missile);
  missile_phys_c.velocity = cam_c.GetLookDir();
  missile_phys_c.is_airborne = true;

  auto& sphere = registry.assign<physics::Sphere>(missile);
  sphere.radius = 0.5f;
  registry.assign<ProjectileComponent>(missile, ProjectileID::MISSILE_OBJECT,
                                       player_c.client_id);

  EventInfo e;
  e.event = Event::CREATE_MISSILE;
  e.entity = missile;
  dispatcher.enqueue<EventInfo>(e);
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

};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
