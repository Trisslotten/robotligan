#ifndef PLAYER_CONTROLLER_SYSTEM_HPP_
#define PLAYER_CONTROLLER_SYSTEM_HPP_

#include <camera_component.hpp>
#include <ecs\components\player_component.hpp>
#include <entt.hpp>
#include "ability_component.hpp"
#include "physics_component.hpp"
#include "transform_component.hpp"
#include <glm/gtx/rotate_vector.hpp>

namespace player_controller {

void Update(entt::registry& registry, float dt) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent,
                    PhysicsComponent, AbilityComponent>();

  for (auto entity : view_controller) {
    CameraComponent& cam_c = view_controller.get<CameraComponent>(entity);
    PlayerComponent& player_c = view_controller.get<PlayerComponent>(entity);
    TransformComponent& trans_c = view_controller.get<TransformComponent>(entity);
    PhysicsComponent& physics_c = view_controller.get<PhysicsComponent>(entity);
    AbilityComponent& ability_c = view_controller.get<AbilityComponent>(entity);

    // rotation
    float sensitivity = 0.003f;
    // rotation this frame from mouse move
    glm::vec2 rot{0};  // = Input::MouseMov() * sensitivity;
    float yaw = rot.x;
    float pitch = rot.y;

    cam_c.AddAngles(yaw, pitch);
    trans_c.Rotate(glm::vec3(0, -yaw, 0));

    if (player_c.actions[SHOOT]) {
      ability_c.shoot = true;
    }
    // Caputre keyboard input and apply velocity

    glm::vec3 final_velocity =
        glm::vec3(0.f, physics_c.velocity.y, 0.f);  //(0, 0, 0);
    glm::vec3 accum_velocity = glm::vec3(0.f);

    // base movement direction on camera orientation.
    glm::vec3 frwd = cam_c.LookDirection();
    // transform_helper::DirVectorFromRadians(cam_c.yaw_, cam_c.pitch_);

    /*
    if (Input::IsKeyPressed(GLFW_KEY_N)) {
      player_c.no_clip = !player_c.no_clip;
      if (player_c.no_clip) {
        physics_c.is_airborne = false;
        physics_c.velocity = glm::vec3(0);
      }
    }
    */

    // we don't want the player to fly if no clip is disabled.
    if (!player_c.no_clip) {
      frwd.y = 0;
      frwd = glm::normalize(frwd);  // renormalize, otherwize done
                                    // in DirVectorFromRadians
    }

    glm::vec3 up(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(frwd, up));

    if (true) {  // abs(accum_velocity.length()) < player_c.walkspeed * 4) {
      if (player_c.actions[WALK_FORWARD]) {
        accum_velocity += frwd;
      }
      if (player_c.actions[WALK_BACKWARD]) {
        accum_velocity -= frwd;
      }
      if (player_c.actions[WALK_RIGHT]) {
        accum_velocity += right;
      }
      if (player_c.actions[WALK_LEFT]) {
        accum_velocity -= right;
      }
      if (glm::length(accum_velocity) > 0.f)
        accum_velocity = glm::normalize(accum_velocity) * player_c.walkspeed;
      if (player_c.actions[SPRINT] &&
          player_c.energy_current > player_c.cost_sprint) {
        accum_velocity *= 2.f;
        player_c.energy_current -= player_c.cost_sprint;
      }
    }

    // physics stuff

    final_velocity += accum_velocity;

    physics_c.velocity = final_velocity;

    glm::vec3 cur_move_dir = glm::normalize(physics_c.velocity);

    // IF player is pressing space
    // AND is not airborne
    // AND has more enery than the cost for jumping
    if (player_c.actions[JUMP] && !physics_c.is_airborne &&
        player_c.energy_current > player_c.cost_jump && !player_c.no_clip) {
      // Add velocity upwards
      final_velocity += up * player_c.jump_speed;
      // Set them to be airborne
      physics_c.is_airborne = true;
      // Subtract energy cost from resources
      player_c.energy_current -= player_c.cost_jump;
    }

    player_c.energy_current =
        std::min((player_c.energy_current + player_c.energy_regen_tick),
                 player_c.energy_max);

    // physics stuff, absolute atm, may need to change. Other
    // systems may affect velocity. velocity of player object.
    physics_c.velocity = final_velocity;

    // slowdown
    glm::vec3 sidemov =
        glm::vec3(physics_c.velocity.x, 0, physics_c.velocity.z);
    float cur_move_speed = glm::length(sidemov);
    if (cur_move_speed > 0.f) {
      physics_c.velocity.x *= 0.95f;
      physics_c.velocity.z *= 0.95f;
    }
    cur_move_speed = physics_c.velocity.length();
    if (cur_move_speed < 0.1f) {
      // physics_c.velocity = glm::vec3(0, 0, 0);
    }

    // Ability buttons
    if (player_c.actions[ABILITY_PRIMARY]) {
      ability_c.use_primary = true;
    }
    if (player_c.actions[ABILITY_SECONDARY]) {
      ability_c.use_secondary = true;
    }

    // std::cout << "pos: " << trans_c.position.x << " " << trans_c.position.y
    //          << " " << trans_c.position.z << "\n";

    // std::cout << "stam: " << player_c.energy_current << "\n";

    // kick ball
    if (player_c.actions[KICK]) {
      glm::vec3 kick_dir =
          cam_c.LookDirection() + glm::vec3(0, player_c.kick_pitch, 0);

      auto view_balls =
          registry.view<BallComponent, PhysicsComponent, TransformComponent>();

      for (auto entity : view_balls) {
        auto& ball_physics_c = view_balls.get<PhysicsComponent>(entity);
        auto& ball_trans_c = view_balls.get<TransformComponent>(entity);
        auto& ball_c = view_balls.get<BallComponent>(entity);
        

        glm::vec3 player_ball_vec = ball_trans_c.position - trans_c.position;
        glm::vec3 player_ball_dir = glm::normalize(player_ball_vec);
        glm::vec3 player_look_dir =
            trans_c.Forward();  // cam_c.LookDirection();
        float dist = length(player_ball_vec);
        float dot = glm::dot(player_look_dir, player_ball_dir);
        if (dist < player_c.kick_reach &&
            dot > player_c.kick_fov) {  // if player is close enough to ball and
                                        // looking at it
          // perform kick
          ball_physics_c.velocity += kick_dir * player_c.kick_force;
          ball_physics_c.is_airborne = true;
        }
      }
    }

    /*
            NETWORK STUFF?
    cam_c.cam->SetPosition(trans_c.position +
                           glm::rotate(cam_c.offset, -trans_c.rotation.y,
                                       glm::vec3(0.0f, 1.0f, 0.0f)));
    */
    //cam_c.cam->SetPosition(trans_c.position + glm::rotate(cam_c.offset, glm::angle(trans_c.rotation), glm::vec3(0.0f, 1.0f, 0.0f)));
  };
}

};  // namespace player_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_HPP_
