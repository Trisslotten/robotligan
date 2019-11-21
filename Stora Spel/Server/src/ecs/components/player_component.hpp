#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

#include <bitset>
#include <shared/shared.hpp>
#include "util/global_settings.hpp"
#include "util/timer.hpp"

struct PlayerComponent {  // Server side
  long client_id = -1;
  float walkspeed = GlobalSettings::Access()->ValueOf("PLAYER_SPEED_WALK");
  float jump_speed = GlobalSettings::Access()->ValueOf("PLAYER_SPEED_JUMP");
  bool no_clip = false;

  // "Stamina"
  float energy_max = GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MAX");
  float energy_current = GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MAX");
  float energy_min_sprint =
      GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_MIN_SPRINT");
  float cost_jump = GlobalSettings::Access()->ValueOf("PLAYER_COST_JUMP");
  float cost_sprint = GlobalSettings::Access()->ValueOf("PLAYER_COST_SPRINT");
  float energy_regen_tick =
      GlobalSettings::Access()->ValueOf("PLAYER_ENERGY_REGEN_TICK");

  // Kicking values
  float kick_cooldown =
      GlobalSettings::Access()->ValueOf("PLAYER_KICK_COOLDOWN");
  float kick_pitch = GlobalSettings::Access()->ValueOf("PLAYER_KICK_PITCH");
  float kick_reach = GlobalSettings::Access()->ValueOf("PLAYER_KICK_REACH");
  float kick_fov = GlobalSettings::Access()->ValueOf("PLAYER_KICK_FOV");
  float kick_force = GlobalSettings::Access()->ValueOf("PLAYER_KICK_FORCE");
  float kick_others_force =
      GlobalSettings::Access()->ValueOf("PLAYER_KICK_OTHERS_FORCE");
  float rocket_speed = GlobalSettings::Access()->ValueOf("PLAYER_ROCKET_SPEED");
  Timer kick_timer;

  // input from client
  std::bitset<PlayerAction::NUM_ACTIONS> actions;
  float yaw = 0;
  float pitch = 0;

  glm::vec3 wanted_move_dir;

  EntityID target = -1;

  // States
  bool sprinting = false;
  bool running = false;
  bool invisible = false;
  float invisibility_remaining = 0.0f;

  bool can_jump = false;
  bool ready_to_smash = false;
  bool stunned = false;
  Timer stun_timer;
  float stun_time = 2.0f;
  // Comparasion Operators
  bool operator==(const PlayerComponent& rhs) {
    return (this->client_id == rhs.client_id) &&
           (this->walkspeed == rhs.walkspeed) &&
           (this->jump_speed == rhs.jump_speed) &&
           (this->no_clip == rhs.no_clip) &&
           (this->energy_max == rhs.energy_max) &&
           (this->energy_current == rhs.energy_current) &&
           (this->cost_jump == rhs.cost_jump) &&
           (this->cost_sprint == rhs.cost_sprint) &&
           (this->energy_regen_tick == rhs.energy_regen_tick) &&
           (this->kick_pitch == rhs.kick_pitch) &&
           (this->kick_reach == rhs.kick_reach) &&
           (this->kick_fov == rhs.kick_fov) &&
           (this->kick_force == rhs.kick_force) &&
           (this->actions == rhs.actions) && (this->yaw == rhs.yaw) &&
           (this->pitch == rhs.pitch) && (this->target == rhs.target) &&
           (this->sprinting == rhs.sprinting) &&
           (this->running == rhs.running) && (this->stunned = rhs.stunned);
		&& (this->ready_to_smash == rhs.ready_to_smash);
  }

  bool operator!=(const PlayerComponent& rhs) { return !((*this) == rhs); }
};

#endif  // PLAYER_COMPONENT_H_