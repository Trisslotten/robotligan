#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

struct PlayerComponent {
  int id = 0;
  float walkspeed = 6.f;
  float jump_speed = 1.0f;
  bool no_clip = false;

  //"Stamina"
  int energy_max = 100;
  int energy_current = 100;
  int cost_jump = 10;

  float kick_pitch = .3f;
  float kick_reach = 1.0f;
  float kick_fov = 0.6f;
  float kick_force = 2.0f;
};

#endif  // PLAYER_COMPONENT_H_