#ifndef PLAYER_COMPONENT_HPP_
#define PLAYER_COMPONENT_HPP_

struct PlayerComponent {
  int id = 0;
  float walkspeed = 2.f;
  bool no_clip = false;
  float kick_pitch = .3f;
  float kick_reach = 1.0f;
  float kick_fov = 0.6f;
  float kick_force = 2.0f;
};

#endif  // PLAYER_COMPONENT_H_