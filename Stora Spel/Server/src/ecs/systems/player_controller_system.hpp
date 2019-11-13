#ifndef PLAYER_CONTROLLER_SYSTEM_HPP_
#define PLAYER_CONTROLLER_SYSTEM_HPP_

#include <entt.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <shared/physics_component.hpp>
#include "ecs/components/ability_component.hpp"
#include "ecs/components/player_component.hpp"
#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"

namespace player_controller {

void foo() {}

// In activate player controller
void InActivatePlayerController() { std::cout << "Escaped pressed\n"; }

void Update(entt::registry& registry, float dt) {
  auto view_controller =
      registry.view<CameraComponent, PlayerComponent, TransformComponent,
                    PhysicsComponent, AbilityComponent, IDComponent>();

  for (auto entity : view_controller) {
    CameraComponent& cam_c = view_controller.get<CameraComponent>(entity);
    PlayerComponent& player_c = view_controller.get<PlayerComponent>(entity);
    TransformComponent& trans_c =
        view_controller.get<TransformComponent>(entity);
    PhysicsComponent& physics_c = view_controller.get<PhysicsComponent>(entity);
    AbilityComponent& ability_c = view_controller.get<AbilityComponent>(entity);
    IDComponent& id_c = view_controller.get<IDComponent>(entity);

    unsigned int player_team = TEAM_RED;
    if (registry.has<TeamComponent>(entity)) {
      player_team = registry.get<TeamComponent>(entity).team;
    }

    constexpr float pi = glm::pi<float>();
    player_c.pitch = glm::clamp(player_c.pitch, -0.49f * pi, 0.49f * pi);
    // player_c.pitch = glm::mod(player_c.pitch + delta_pitch, 2.f * pi);
    // player_c.pitch += delta_pitch;
    cam_c.orientation = glm::quat(glm::vec3(0, player_c.yaw, 0)) *
                        glm::quat(glm::vec3(0, 0, player_c.pitch));
    cam_c.orientation = glm::normalize(cam_c.orientation);
    trans_c.SetRotation(glm::vec3(0, player_c.yaw, 0));

    if (player_c.actions[PlayerAction::SHOOT]) {
      GameEvent shoot_event;
      shoot_event.type = GameEvent::SHOOT;
      shoot_event.shoot.player_id = registry.get<IDComponent>(entity).id;
      dispatcher.trigger(shoot_event);
      ability_c.shoot = true;
    }
    // Caputre keyboard input and apply velocity

    glm::vec3 final_velocity =
        glm::vec3(0.f, physics_c.velocity.y, 0.f);  //(0, 0, 0);
    glm::vec3 accum_velocity = glm::vec3(0.f);

    // base movement direction on camera orientation.
    glm::vec3 frwd = cam_c.GetLookDir();

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
      if (player_c.actions[PlayerAction::WALK_FORWARD] ||
          player_c.actions[PlayerAction::WALK_BACKWARD] ||
          player_c.actions[PlayerAction::WALK_RIGHT] ||
          player_c.actions[PlayerAction::WALK_LEFT]) {
        if (!player_c.running) {
          GameEvent run_event;
          run_event.type = GameEvent::RUN_START;
          run_event.sprint_start.player_id =
              registry.get<IDComponent>(entity).id;
          dispatcher.trigger(run_event);
          player_c.running = true;
        }
        if (player_c.actions[PlayerAction::WALK_FORWARD]) {
          accum_velocity += frwd;
        }
        if (player_c.actions[PlayerAction::WALK_BACKWARD]) {
          accum_velocity -= frwd;
        }
        if (player_c.actions[PlayerAction::WALK_RIGHT]) {
          accum_velocity += right;
        }
        if (player_c.actions[PlayerAction::WALK_LEFT]) {
          accum_velocity -= right;
        }
      } else if (player_c.running) {
        GameEvent run_event;
        run_event.type = GameEvent::RUN_END;
        run_event.sprint_start.player_id = registry.get<IDComponent>(entity).id;
        dispatcher.trigger(run_event);
        player_c.running = false;
      }
      if (glm::length(accum_velocity) > 0.f) {
        accum_velocity = glm::normalize(accum_velocity) * player_c.walkspeed;
      }

      if (player_c.actions[PlayerAction::SPRINT]) {
        if (!player_c.sprinting) {
          GameEvent sprint_event;
          sprint_event.type = GameEvent::SPRINT_START;
          sprint_event.sprint_start.player_id =
              registry.get<IDComponent>(entity).id;
          dispatcher.trigger(sprint_event);
        }
        player_c.sprinting = true;

        if (player_c.energy_current > player_c.cost_sprint * dt) {
          accum_velocity *= 2.f;
          player_c.energy_current -= player_c.cost_sprint * dt;
        }
      } else if (!player_c.actions[PlayerAction::SPRINT] &&
                 player_c.sprinting) {
        player_c.sprinting = false;
        GameEvent sprint_event;
        sprint_event.type = GameEvent::SPRINT_END;
        sprint_event.sprint_end.player_id =
            registry.get<IDComponent>(entity).id;
        dispatcher.trigger(sprint_event);
      }
    }

    player_c.wanted_move_dir = accum_velocity;

    // physics stuff

    final_velocity += accum_velocity;

    // physics_c.velocity = final_velocity;

    glm::vec3 cur_move_dir = glm::normalize(physics_c.velocity);

    // IF player is pressing space
    // AND is not airborne
    // AND has more enery than the cost for jumping
    if (player_c.actions[PlayerAction::JUMP] && player_c.can_jump &&
        player_c.energy_current > player_c.cost_jump && !player_c.no_clip) {
      // Add velocity upwards
      final_velocity += up * player_c.jump_speed;
      // Set them to be airborne
      player_c.can_jump = false;
      // Subtract energy cost from resources
      player_c.energy_current -= player_c.cost_jump;

      // save game event
      if (registry.has<IDComponent>(entity)) {
        GameEvent jump_event;
        jump_event.type = GameEvent::JUMP;
        jump_event.jump.player_id = registry.get<IDComponent>(entity).id;
        dispatcher.trigger(jump_event);
      }
    }

    player_c.energy_current =
        std::min((player_c.energy_current + player_c.energy_regen_tick * dt),
                 player_c.energy_max);

    // slowdown
    glm::vec3 sidemov = glm::vec3(final_velocity.x, 0, final_velocity.z);
    float cur_move_speed = glm::length(sidemov);
    // if (cur_move_speed > 0.f) {
    // movement "floatiness", lower value = less floaty
    float t = 0.0005f;
    physics_c.velocity.x =
        glm::mix(physics_c.velocity.x, final_velocity.x, 1.f - glm::pow(t, dt));
    physics_c.velocity.z =
        glm::mix(physics_c.velocity.z, final_velocity.z, 1.f - glm::pow(t, dt));
    //}
    physics_c.velocity.y = final_velocity.y;

    // physics stuff, absolute atm, may need to change. Other
    // systems may affect velocity. velocity of player object.

    // Ability buttons
    if (player_c.actions[PlayerAction::ABILITY_PRIMARY]) {
      ability_c.use_primary = true;
    }
    if (player_c.actions[PlayerAction::ABILITY_SECONDARY]) {
      ability_c.use_secondary = true;
    }

    // std::cout << "pos: " << trans_c.position.x << " " << trans_c.position.y
    //          << " " << trans_c.position.z << "\n";

    // std::cout << "stam: " << player_c.energy_current << "\n";

    // kick ball
    if (player_c.actions[PlayerAction::KICK] &&
        player_c.kick_timer.Elapsed() > player_c.kick_cooldown) {
      player_c.kick_timer.Restart();
      GameEvent kick_event;
      kick_event.type = GameEvent::KICK;
      kick_event.kick.player_id = id_c.id;
      dispatcher.trigger(kick_event);

      glm::vec3 kick_dir =
          cam_c.GetLookDir() + glm::vec3(0, player_c.kick_pitch, 0);

      auto view_balls =
          registry.view<BallComponent, PhysicsComponent, TransformComponent>();

      for (auto entity : view_balls) {
        auto& ball_physics_c = view_balls.get<PhysicsComponent>(entity);
        auto& ball_trans_c = view_balls.get<TransformComponent>(entity);
        auto& ball_c = view_balls.get<BallComponent>(entity);

        glm::vec3 player_ball_vec = ball_trans_c.position - trans_c.position;
        glm::vec3 player_ball_dir = glm::normalize(player_ball_vec);
        glm::vec3 player_look_dir = cam_c.GetLookDir();
        float dist = length(player_ball_vec);
        float dot = glm::dot(player_look_dir, player_ball_dir);
        if (dist < player_c.kick_reach &&
            dot > player_c.kick_fov) {  // if player is close enough to ball and
                                        // looking at it
          // perform kick
          kick_dir = glm::normalize(kick_dir);
          ball_physics_c.velocity = kick_dir * player_c.kick_force;
          ball_physics_c.is_airborne = true;
          ball_c.last_touch = player_c.client_id;

          // save game event
          GameEvent hit_event;
          hit_event.type = GameEvent::HIT;
          hit_event.hit.player_id = id_c.id;
          dispatcher.trigger(hit_event);
          if (!ball_c.is_real && ball_c.faker_team != player_team) {
            EventInfo info;
            if (registry.has<IDComponent>(entity) == false) return;
            auto id = registry.get<IDComponent>(entity);
            info.event = Event::DESTROY_ENTITY;
            info.e_id = id.id;
            info.entity = entity;
            dispatcher.enqueue<EventInfo>(info);
          }
        }
      }

      auto view_others =
          registry
              .view<PlayerComponent, TransformComponent, PhysicsComponent>();
      for (auto other : view_others) {
        auto& other_phys_c = view_others.get<PhysicsComponent>(other);
        auto& other_trans_c = view_others.get<TransformComponent>(other);
        auto& other_player_c = view_others.get<PlayerComponent>(other);

        glm::vec3 player_other_vec = other_trans_c.position - trans_c.position;
        glm::vec3 player_other_dir = glm::normalize(player_other_vec);
        glm::vec3 player_look_dir = cam_c.GetLookDir();
        float dist = length(player_other_vec);
        float dot = glm::dot(player_look_dir, player_other_dir);
        if (dist < player_c.kick_reach && dot > player_c.kick_fov) {
          // perform kick
          other_phys_c.velocity += kick_dir * player_c.kick_others_force;
          other_phys_c.is_airborne = true;

          // save game event
          GameEvent hit_event;
          hit_event.type = GameEvent::HIT;
          hit_event.hit.player_id = id_c.id;
          dispatcher.trigger(hit_event);
        }
      }
    }

    /*
            NETWORK STUFF?
    cam_c.cam->SetPosition(trans_c.position +
                           glm::rotate(cam_c.offset, -trans_c.rotation.y,
                                       glm::vec3(0.0f, 1.0f, 0.0f)));
    */
    // cam_c.cam->SetPosition(trans_c.position + glm::rotate(cam_c.offset,
    // glm::angle(trans_c.rotation), glm::vec3(0.0f, 1.0f, 0.0f)));
  };
}

};  // namespace player_controller

#endif  // !PLAYER_CONTROLLER_SYSTEM_HPP_
