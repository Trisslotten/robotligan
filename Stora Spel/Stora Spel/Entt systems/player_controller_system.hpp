#ifndef PLAYER_CONTROLLER_SYSTEM_HPP_
#define PLAYER_CONTROLLER_SYSTEM_HPP_

#include <entt.hpp>
#include "../util/input.hpp"
#include "camera_component.hpp"
#include "physics_component.hpp"
#include "player_component.hpp"
#include "transform_component.hpp"

namespace player_controller {

void foo() {}

void Update(entt::registry& registry, float dt) {
  foo();

  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent,
                    PhysicsComponent, AbilityComponent>();

  for (auto entity : view_controller) {
    CameraComponent& cam_c = view_controller.get<CameraComponent>(entity);
    PlayerComponent& player_c = view_controller.get<PlayerComponent>(entity);
    TransformComponent& trans_c =
        view_controller.get<TransformComponent>(entity);
    PhysicsComponent& physics_c = view_controller.get<PhysicsComponent>(entity);
    AbilityComponent& ability_c = view_controller.get<AbilityComponent>(entity);
    // rotation
    float sensitivity = 0.003f;
    glm::vec2 rot =
        Input::MouseMov() * sensitivity;  // rotation this frame from mouse move
    float yaw = rot.x;
    float pitch = rot.y;

    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
      cam_c.AddAngles(yaw, pitch);
      trans_c.Rotate(glm::vec3(0, yaw, 0));
    }

    // Caputre keyboard input and apply velocity

    glm::vec3 final_velocity(0, 0, 0);

    // base movement direction on camera orientation.
    glm::vec3 frwd = cam_c.LookDirection();
    // transform_helper::DirVectorFromRadians(cc.yaw_, cc.pitch_);

    if (Input::IsKeyPressed(GLFW_KEY_N)) {
      player_c.no_clip = !player_c.no_clip;
    }

    // we don't want the player to fly if no clip is disabled.
    if (!player_c.no_clip) {
      frwd.y = 0;
      frwd = glm::normalize(frwd);  // renormalize, otherwize done
                                    // in DirVectorFromRadians
    }

    glm::vec3 up(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(frwd, up));

    if (Input::IsKeyDown(GLFW_KEY_W)) {
      final_velocity += frwd * player_c.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_S)) {
      final_velocity -= frwd * player_c.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_D)) {
      final_velocity += right * player_c.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_A)) {
      final_velocity -= right * player_c.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
      final_velocity *= 2;
    }
    if (Input::IsKeyDown(GLFW_KEY_SPACE) && !physics_c.airborne) {
      final_velocity += up * player_c.jump_speed;
      physics_c.airborne = true;
    }
    // physics stuff, absolute atm, may need to change. Other
    // systems may affect velocity. velocity of player object.
    physics_c.velocity = final_velocity;

    // Ability buttons
    if (Input::IsKeyDown(GLFW_KEY_Q)) {
      ability_c.use_primary = true;
    }
    if (Input::IsKeyDown(GLFW_KEY_E)) {
      ability_c.use_primary = true;
    }

    /*
            NETWORK STUFF?
    */

    // maybe move to new CameraSystem?
    cam_c.cam->SetPosition(trans_c.position + cam_c.offset);
    // maybe move to new CameraSystem?
  };
}

};  // namespace player_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_HPP_
