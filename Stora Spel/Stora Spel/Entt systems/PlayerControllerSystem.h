#ifndef PLAYER_CONTROLLER_SYSTEM_H_
#define PLAYER_CONTROLLER_SYSTEM_H_

#include <entt.hpp>
#include "camera_component.h"
#include "player_component.h"
#include "transform_component.h"
#include "util/input.h"
#include <util/ability.hpp>

namespace p_controller {

void update(entt::registry &registry) {
  registry.view<CameraComponent, PlayerComponent, TransformComponent>().each(
      [&](CameraComponent &cc, PlayerComponent &pc, TransformComponent &tc) {
        // rotation
        float sensitivity = 1.0f;
        glm::vec2 rot = Input::mouseMov() * sensitivity;
        float yaw = rot.x;
        float pitch = rot.y;
        tc.rotation.x = pitch;
        tc.rotation.y += yaw;
        tc.rotation = glm::clamp(tc.rotation, -180.f, 180.f);

        if (Input::isKeyDown(GLFW_KEY_W)) {
          // velocity stuff, handle acc here and let PhysicsSystem handle
          // movement and collisions?
        }
        if (Input::isKeyDown(GLFW_KEY_E)) {
          // Trigger ability func, send in player as parameter
          Ability::TriggerAbility(registry, pc);
        }

        // maybe move to new CameraSystem?
        cc.cam->TurnCameraViaDegrees(yaw, pitch * -1);
        cc.cam->MoveCamera(tc.position + cc.offset);
        // maybe move to new CameraSystem?
      });
}

};  // namespace p_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_H_
