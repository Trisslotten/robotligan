#ifndef BUFF_CONTROLLER_SYSTEM_HPP_
#define BUFF_CONTROLLER_SYSTEM_HPP_

//#include <position.h>
#include <../util/global_settings.hpp>
//#include <boundingboxes.hpp>
//#include <camera_component.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"

namespace buff_controller {

bool TriggerBuff(entt::registry &regitry, BuffID in_b_id);
void ApplySpeedBoost(entt::registry &registry);
void ApplyJumpBoost(entt::registry &registry);
void ApplyInfiniteStamina(entt::registry &registry);
// ^^^          Add more buff functions above             ^^^
void ApplyStandardValues(entt::registry &registry);
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
          ApplyStandardValues(registry);
        }

        // Check if any buffs toggled AND buff is not currently active
        if (buff_component.buff_toggle &&
            !(buff_component.duration_remaining > 0.0f)) {
          // Trigger the buff
          if (TriggerBuff(registry, buff_component.active_buff)) {
            buff_component.duration_remaining = buff_component.duration;
          }
        }

        // When buff is toggled, set the buff toggle back to false
        buff_component.buff_toggle = false;
      });
}

bool TriggerBuff(entt::registry &registry, BuffID in_b_id) {
  switch (in_b_id) {
    case NULL_BUFF:
      break;
    case SPEED_BOOST:
      ApplySpeedBoost(registry);
      return true;
      break;
    case JUMP_BOOST:
      ApplyJumpBoost(registry);
      return true;
      break;
    case INFINITE_STAMINA:
      ApplyInfiniteStamina(registry);
      return true;
      break;
    default:
      return false;
      break;
  }
}

void ApplySpeedBoost(entt::registry &registry) {
  auto player_view = registry.view<PlayerComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent &player_c = player_view.get(player_entity);

    player_c.walkspeed +=
        GlobalSettings::Access()->ValueOf("BUFF_SPEED_BOOST_VALUE");
  }
}

void ApplyJumpBoost(entt::registry &registry) {
  auto player_view = registry.view<PlayerComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent &player_c = player_view.get(player_entity);

    player_c.jump_speed +=
        GlobalSettings::Access()->ValueOf("BUFF_JUMP_BOOST_VALUE");
  }
}

void ApplyInfiniteStamina(entt::registry &registry) {
  auto player_view = registry.view<PlayerComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent &player_c = player_view.get(player_entity);

    player_c.cost_jump = 0.0f;
    player_c.cost_sprint = 0.0f;
  }
}

void ApplyStandardValues(entt::registry &registry) {
  auto player_view = registry.view<PlayerComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent &player_c = player_view.get(player_entity);
    // APPLY STANDARD VALUES TO ALL AFFECTED PLAYER STATS
    player_c.walkspeed = GlobalSettings::Access()->ValueOf("PLAYER_SPEED_WALK");
    player_c.jump_speed =
        GlobalSettings::Access()->ValueOf("PLAYER_SPEED_JUMP");
    player_c.cost_jump = GlobalSettings::Access()->ValueOf("PLAYER_COST_JUMP");
    player_c.cost_sprint =
        GlobalSettings::Access()->ValueOf("PLAYER_COST_SPRINT");
  }
}

};  // namespace buff_controller

#endif  // !BUFF_CONTROLLER_SYSTEM_HPP_