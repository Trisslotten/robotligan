#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

struct PlayerComponent {
  int id = 0;
  float walkspeed = 1.f;
  float jump_speed = 1.0f;
  bool no_clip = false;

  //"Stamina"
  int energy_max = 100;
  int energy_current = 100;
  int cost_jump = 10;

};

#endif  // PLAYER_COMPONENT_H_