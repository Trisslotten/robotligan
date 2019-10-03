#ifndef BUFF_CONTROLLER_SYSTEM_HPP_
#define BUFF_CONTROLLER_SYSTEM_HPP_

//#include <position.h>
#include <../util/global_settings.hpp>
//#include <boundingboxes.hpp>
//#include <camera_component.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"

namespace buff_controller {

bool TriggerBuff(PlayerComponent &in_player_component, BuffID in_b_id);
void ApplySpeedBoost(PlayerComponent &in_player_component);
void ApplyJumpBoost(PlayerComponent &in_player_component);
void ApplyInfiniteStamina(PlayerComponent &in_player_component);
// ^^^          Add more buff functions above             ^^^
void EndBuff(PlayerComponent &in_player_component, BuffID in_b_id);
// ^^^ Don't forget to reset values affected by new buffs ^^^

void Update(entt::registry &registry, float dt) {
  registry.view<PlayerComponent, BuffComponent>().each(
      [&](PlayerComponent &player_component, BuffComponent &buff_component) {
        // Loop over each entity with a PlayerComponent
        // and Buff Component

        // Update durations
        buff_component.duration_remaining -= dt;
        if (buff_component.duration_remaining < 0.0f) {
          buff_component.duration_remaining = 0.0f;
          EndBuff(player_component, buff_component.active_buff);
        }

        // Check if any buffs toggled AND buff is not currently active
        if (buff_component.buff_toggle &&
            !(buff_component.duration_remaining > 0.0f)) {
          // Trigger the buff
          if (TriggerBuff(player_component, buff_component.active_buff)) {
            buff_component.duration_remaining = buff_component.duration;
          }
        }

        // When buff is toggled, set the buff toggle back to false
        buff_component.buff_toggle = false;
      });
}

bool TriggerBuff(PlayerComponent &in_player_component, BuffID in_b_id) {
  switch (in_b_id) {
    case NULL_BUFF:
      break;
    case SPEED_BOOST:
      ApplySpeedBoost(in_player_component);
      return true;
      break;
    case JUMP_BOOST:
      ApplyJumpBoost(in_player_component);
      return true;
      break;
    case INFINITE_STAMINA:
      ApplyInfiniteStamina(in_player_component);
      return true;
      break;
    default:
      return false;
      break;
  }
}

void ApplySpeedBoost(PlayerComponent &in_player_component) {
  in_player_component.walkspeed +=
      GlobalSettings::Access()->ValueOf("BUFF_SPEED_BOOST_VALUE");
}

void ApplyJumpBoost(PlayerComponent &in_player_component) {
  in_player_component.jump_speed +=
      GlobalSettings::Access()->ValueOf("BUFF_JUMP_BOOST_VALUE");
}

void ApplyInfiniteStamina(PlayerComponent &in_player_component) {
  in_player_component.cost_jump = 0.0f;
  in_player_component.cost_sprint = 0.0f;
}

void EndBuff(PlayerComponent &in_player_component, BuffID in_b_id) {
  switch (in_b_id) {
    case NULL_BUFF:
      break;
    case SPEED_BOOST:
      in_player_component.walkspeed =
          GlobalSettings::Access()->ValueOf("PLAYER_SPEED_WALK");
      break;
    case JUMP_BOOST:
      in_player_component.jump_speed =
          GlobalSettings::Access()->ValueOf("PLAYER_SPEED_JUMP");
      break;
    case INFINITE_STAMINA:
      in_player_component.cost_jump =
          GlobalSettings::Access()->ValueOf("PLAYER_COST_JUMP");
      in_player_component.cost_sprint =
          GlobalSettings::Access()->ValueOf("PLAYER_COST_SPRINT");
      break;
    default:
      break;
  }
}
};  // namespace buff_controller

#endif  // !BUFF_CONTROLLER_SYSTEM_HPP_