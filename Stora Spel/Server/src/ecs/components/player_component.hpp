#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include <bitset>
#include <shared.hpp>
#include "..//util/global_settings.hpp"

struct PlayerComponent {
  PlayerID id = -1;
  float walkspeed = GlobalSettings::Access()->ValueOf("PLAYER_SPEED_WALK");
  float jump_speed = GlobalSettings::Access()->ValueOf("PLAYER_SPEED_JUMP");
  bool no_clip = false;

  // "Stamina"
  float energy_max = GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MAX");
  float energy_current =GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MAX");
  float cost_jump = GlobalSettings::Access()->ValueOf("PLAYER_COST_JUMP");
  float cost_sprint = GlobalSettings::Access()->ValueOf("PLAYER_COST_SPRINT");
  float energy_regen_tick = GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_REGEN_TICK");

  // Kicking values
  float kick_pitch = GlobalSettings::Access()->ValueOf("PLAYER_KICK_PITCH");
  float kick_reach = GlobalSettings::Access()->ValueOf("PLAYER_KICK_REACH");
  float kick_fov = GlobalSettings::Access()->ValueOf("PLAYER_KICK_FOV");
  float kick_force = GlobalSettings::Access()->ValueOf("PLAYER_KICK_FORCE");

  // input from client
  std::bitset<PlayerAction::NUM_ACTIONS> actions;
  float yaw = 0;
  float pitch = 0;
};

#endif  // PLAYER_COMPONENT_H_