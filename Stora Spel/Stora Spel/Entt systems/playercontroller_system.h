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
        float sensitivity = 1.0f;
        glm::vec2 rot = Input::mouseMov() *
                        sensitivity;  // rotation this frame from mouse move
        float yaw = rot.x;
        float pitch = rot.y;

        if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
          cc.AddAngles(yaw, pitch);
          tc.rotate(glm::vec3(pitch, 0, yaw));
        }

        // tc.rotation.x += pitch;
        // tc.rotation.z += yaw;

        glm::vec3 final_velocity(0, 0, 0);

        if (Input::isKeyDown(GLFW_KEY_W)) {
          glm::vec3 frwd = tc.Forward();
          final_velocity += frwd * pc.walkSpeed;
          std::cout << "Forward vector: " << frwd.x << " " << frwd.y << " "
                    << frwd.z << "\n";
        }

        // physics stuff
        v.velocity = final_velocity;

        // maybe move to new CameraSystem?
        // cc.cam->TurnCameraViaDegrees(yaw, pitch);
        cc.cam->MoveCamera(tc.position + cc.offset);
        // maybe move to new CameraSystem?
      });
}

};  // namespace p_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_H_
