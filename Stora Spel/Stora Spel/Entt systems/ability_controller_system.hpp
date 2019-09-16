#ifndef ABILITY_CONTROLLER_SYSTEM_HPP_
#define ABILITY_CONTROLLER_SYSTEM_HPP_

//#include <position.h>
#include <ability_component.hpp>
#include <boundingboxes.hpp>
#include <player_component.hpp>
#include <physics_component.hpp>
#include <projectile_component.hpp>
#include <transform_component.hpp>

namespace ability_controller {

void TriggerAbility(entt::registry &registry, AbilityID in_id);
void CreateMissileEntity(entt::registry &registry);
void CreateCannonBallEntity(entt::registry &registry);

void Update(
    entt::registry &registry, float dt) {
  registry.view<PlayerComponent, AbilityComponent>().each(
      [&](PlayerComponent &player_component,
          AbilityComponent &ability_component) {
		//update cooldown
        ability_component.cooldown -= dt;
        ability_component.shoot_cooldown -= dt;
        // First check if primary ability is being used
        // AND ability is not on cooldown
        if (ability_component.use_primary &&
            !(ability_component.cooldown > 0)) {
          TriggerAbility(registry, ability_component.primary_ability);
        }

        // Then check if secondary ability is being used
        if (ability_component.use_secondary) {
          TriggerAbility(registry, ability_component.secondary_ability);
        }

		//Check if the player should shoot
        if (ability_component.shoot && ability_component.shoot_cooldown <= 0.0f) {
          CreateCannonBallEntity(registry);
          ability_component.shoot_cooldown = 10.0f;
		}
        ability_component.shoot = false;
      });
}

void TriggerAbility(entt::registry &registry, AbilityID in_id) {
  switch (in_id) {
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
      break;
    case SUPER_STRIKE:
      break;
    case SWITCH_GOALS:
      break;
    case TELEPORT:
      break;
    default:
      break;
  }
}

void CreateMissileEntity(entt::registry &registry) {
  // Create new entity
  auto entity = registry.create();

  // WIP: Assign it the correct components here
}

void CreateCannonBallEntity(entt::registry& registry) {
  auto view_controller = registry.view<CameraComponent, PlayerComponent,
                                       TransformComponent>();
  
  for (auto entity : view_controller) {
    CameraComponent &cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent &pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent &tc = view_controller.get<TransformComponent>(entity);

    float speed = 500.0f;
    auto cannonball = registry.create();
    registry.assign<PhysicsComponent>(
        cannonball, glm::vec3(cc.LookDirection() * speed), true, 0.0f);
    registry.assign<TransformComponent>(cannonball, glm::vec3(tc.position + cc.offset), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    registry.assign<physics::Sphere>(cannonball, glm::vec3(tc.position + cc.offset), 1.0f);
    registry.assign<ProjectileComponent>(cannonball, CANNON_BALL);
  }
}

};  // namespace ability_controller

#endif  // !ABILITY_CONTROLLER_SYSTEM_HPP_
