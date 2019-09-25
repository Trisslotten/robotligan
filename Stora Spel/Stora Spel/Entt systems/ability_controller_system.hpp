#ifndef ABILITY_CONTROLLER_SYSTEM_HPP_
#define ABILITY_CONTROLLER_SYSTEM_HPP_

//#include <position.h>
#include <../util/global_settings.hpp>
#include <ability_component.hpp>
#include <ball_component.hpp>
#include <boundingboxes.hpp>
#include <camera_component.hpp>
#include <model_component.hpp>
#include <physics_component.hpp>
#include <player_component.hpp>
#include <projectile_component.hpp>
#include <transform_component.hpp>
#include <light_component.hpp>
#include <entt.hpp>

namespace ability_controller {

bool TriggerAbility(entt::registry &registry, AbilityID in_a_id);
void CreateMissileEntity(entt::registry &registry);
void DoSuperStrike(entt::registry &registry);
void CreateCannonBallEntity(entt::registry &registry);

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
          ability_component.cooldown_remaining == 0.0f;
        }
        if (ability_component.shoot_cooldown < 0.0f) {
          ability_component.shoot_cooldown == 0.0f;
        }

        // First check if primary ability is being used
        // AND ability is not on cooldown
        if (ability_component.use_primary &&
            !(ability_component.cooldown_remaining > 0.0f)) {
          // Trigger the ability
          if (TriggerAbility(registry, ability_component.primary_ability)) {
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
          if (TriggerAbility(registry, ability_component.secondary_ability)) {
            // If ability triggered successfully, remove the
            // slotted secondary ability
            ability_component.secondary_ability = NULL_ABILITY;
          }
        }
        // When finished set secondary ability to not activated
        ability_component.use_secondary = false;

        // Check if the player should shoot
        if (ability_component.shoot &&
            ability_component.shoot_cooldown <= 0.0f) {
          CreateCannonBallEntity(registry);
          ability_component.shoot_cooldown = 1.0f;
        }
        ability_component.shoot = false;
      });
}

bool TriggerAbility(entt::registry &registry, AbilityID in_a_id) {
  switch (in_a_id) {
    case NULL_ABILITY:
      break;
    case BUILD_WALL:
      break;
    case FAKE_BALL:
      break;
    case FORCE_PUSH:
      break;
    case GRAVITY_CHANGE:
      break;
    case HOMING_BALL:
      break;
    case INVISIBILITY:
      break;
    case MISSILE:
      CreateMissileEntity(registry);
      return true;
      break;
    case SUPER_STRIKE:
      DoSuperStrike(registry);
      return true;
      break;
    case SWITCH_GOALS:
      break;
    case TELEPORT:
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
      glm::vec3 player_look_dir = camera_c.LookDirection();

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
            camera_c.LookDirection() + glm::vec3(0, player_c.kick_pitch, 0);

        // Apply the force of the kick to the ball's velocity
        physics_c_ball.velocity += kick_dir * GlobalSettings::Access()->ValueOf(
                                                  "ABILITY_SUPER_STRIKE_FORCE");
        physics_c_ball.is_airborne = true;
      }
    }
  }
}

void CreateCannonBallEntity(entt::registry &registry) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent>();

  for (auto entity : view_controller) {
    CameraComponent &cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent &pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent &tc = view_controller.get<TransformComponent>(entity);

    float speed = 0.0f;
    auto cannonball = registry.create();
    registry.assign<PhysicsComponent>(
        cannonball, glm::vec3(cc.LookDirection() * speed), false, 0.0f);
    registry.assign<TransformComponent>(
        cannonball, glm::vec3(cc.LookDirection() * 1.5f + tc.position + cc.offset),
        glm::vec3(0, 0, 0), glm::vec3(.3f, .3f, .3f));
    registry.assign<physics::Sphere>(cannonball,
                                     glm::vec3(tc.position + cc.offset), .3f);
    registry.assign<ProjectileComponent>(cannonball, CANNON_BALL);
    registry.assign<ModelComponent>(cannonball,
                                    glob::GetModel("assets/Ball/Ball.fbx"));
    registry.assign<LightComponent>(cannonball, glm::vec3(1, 0, 1), 3.f, 0.f);

  }
}

};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
