#ifndef PLAYER_CONTROLLER_SYSTEM_HPP_
#define PLAYER_CONTROLLER_SYSTEM_HPP_

#include <entt.hpp>
#include "../util/input.hpp"
#include "camera_component.hpp"
#include "player_component.hpp"
#include "transform_component.hpp"
#include "velocity_component.hpp"

namespace player_controller {

void foo() {}

void Update(entt::registry& registry, float dt) {
  foo();

  auto view_controller = registry.view<CameraComponent, PlayerComponent,
                                       TransformComponent, VelocityComponent>();

  for (auto entity : view_controller) {
    CameraComponent& cc = view_controller.get<CameraComponent>(entity);
    PlayerComponent& pc = view_controller.get<PlayerComponent>(entity);
    TransformComponent& tc = view_controller.get<TransformComponent>(entity);
    VelocityComponent& vc = view_controller.get<VelocityComponent>(entity);
    // rotation
    float sensitivity = 0.003f;
    glm::vec2 rot =
        Input::MouseMov() * sensitivity;  // rotation this frame from mouse move
    float yaw = rot.x;
    float pitch = rot.y;

    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
      cc.AddAngles(yaw, pitch);
      tc.Rotate(glm::vec3(0, yaw, 0));
    }

    // Caputre keyboard input and apply velocity

    glm::vec3 final_velocity(0, 0, 0);

    // base movement direction on camera orientation.
    glm::vec3 frwd = cc.LookDirection();
    // transform_helper::DirVectorFromRadians(cc.yaw_, cc.pitch_);

    if (Input::IsKeyPressed(GLFW_KEY_N)) {
      pc.no_clip = !pc.no_clip;
    }

    // we don't want the player to fly if no clip is disabled.
    if (!pc.no_clip) {
      frwd.y = 0;
      frwd = glm::normalize(frwd);  // renormalize, otherwize done
                                    // in DirVectorFromRadians
    }

    glm::vec3 up(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(frwd, up));

    if (Input::IsKeyDown(GLFW_KEY_W)) {
      final_velocity += frwd * pc.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_S)) {
      final_velocity -= frwd * pc.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_D)) {
      final_velocity += right * pc.walkspeed * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_A)) {
      final_velocity -= right * pc.walkspeed * dt;
    }

    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
      final_velocity *= 2;
    }

    // physics stuff, absolute atm, may need to change. Other
    // systems may affect velocity. velocity of player object.
    vc.velocity = final_velocity;

    /*
            NETWORK STUFF?
    */

    // maybe move to new CameraSystem?
    cc.cam->SetPosition(tc.position + cc.offset);
    // maybe move to new CameraSystem?
  };
}

};  // namespace player_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_HPP_
