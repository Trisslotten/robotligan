#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

struct PlayerComponent {
  int id = 0;
  float walkspeed = 15.f;
  float jump_speed = 5.0f;
  bool no_clip = false;

  //"Stamina"
  int energy_max = 100;
  int energy_current = 100;
  int cost_jump = 10;
  int cost_sprint = 3;
  int energy_regen_tick = 1;

  float kick_pitch = .3f;
  float kick_reach = 2.0f;
  float kick_fov = 0.6f;
  float kick_force = 20.0f;
};

#endif  // PLAYER_COMPONENT_H_