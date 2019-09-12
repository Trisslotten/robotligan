#ifndef PLAYER_CONTROLLER_SYSTEM_H_
#define PLAYER_CONTROLLER_SYSTEM_H_

#include <entt.hpp>
#include "camera_component.h"
#include "player_component.h"
#include "transform_component.h"
#include "util/input.h"
#include "velocity.h"

namespace p_controller {

void update(entt::registry &registry) {
  registry
      .view<CameraComponent, PlayerComponent, TransformComponent, Velocity>()
      .each([](CameraComponent &cc, PlayerComponent &pc, TransformComponent &tc,
               Velocity &v) {
        // rotation
        float sensitivity = 0.003f;
        glm::vec2 rot = Input::mouseMov() *
                        sensitivity;  // rotation this frame from mouse move
        float yaw = rot.x;
        float pitch = rot.y;

        if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
         
          cc.AddAngles(yaw, pitch);
          tc.rotate(glm::vec3(0, yaw, 0));
        }

        // Caputre keyboard input and apply velocity

        glm::vec3 final_velocity(0, 0, 0);

        glm::vec3 front = tc.Forward();
        front.y = 0;  // we don't want the player to fly
        glm::vec3 up(0, 1, 0);
        glm::vec3 right = glm::cross(front, up);

        if (Input::isKeyDown(GLFW_KEY_W)) {
          final_velocity += front * pc.walkSpeed;
        }
        if (Input::isKeyDown(GLFW_KEY_S)) {
          final_velocity -= front * pc.walkSpeed;
        }
        if (Input::isKeyDown(GLFW_KEY_D)) {
          final_velocity += right * pc.walkSpeed;
        }
        if (Input::isKeyDown(GLFW_KEY_A)) {
          final_velocity -= right * pc.walkSpeed;
        }

        // physics stuff, absolute atm, need to change. Other systems may affect
        // velocity of player object.
        v.velocity = final_velocity;

        /*
                NETWORK STUFF?
        */

        // maybe move to new CameraSystem?
        cc.cam_->SetPosition(tc.position + cc.offset_);
        // maybe move to new CameraSystem?
      });
}

};  // namespace p_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_H_
