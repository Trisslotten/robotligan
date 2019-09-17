#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include "..//util/global_settings.hpp"

struct PlayerComponent {
  int id = 0;
  float walkspeed = GlobalSettings::Access()->ValueOf("PLAYER_SPEeED_WALK");
  float jump_speed = GlobalSettings::Access()->ValueOf("PLAYER_SPEED_JUMP");
  bool no_clip = false;

  // "Stamina"
  int energy_max = (int)GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MAX");
  int energy_current = (int)GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MAX");
  int cost_jump = (int)GlobalSettings::Access()->ValueOf("PLAYER_COST_JUMP");
  int cost_sprint = 3;
  int energy_regen_tick = 1;

  // Kicking values
  float kick_pitch = GlobalSettings::Access()->ValueOf("PLAYER_KICK_PITCH");
  float kick_reach = GlobalSettings::Access()->ValueOf("PLAYER_KICK_REACH");
  float kick_fov = GlobalSettings::Access()->ValueOf("PLAYER_KICK_FOV");
  float kick_force = GlobalSettings::Access()->ValueOf("PLAYER_KICK_FORCE");
};

#endif  // PLAYER_COMPONENT_H_