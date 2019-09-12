#ifndef PLAYER_COMPONENT_H_
#define PLAYER_COMPONENT_H_

struct PlayerComponent {
  int id_ = 0;
  float walkspeed_ = 0.01f;
  bool no_clip_ = false;
};

#endif  // PLAYER_COMPONENT_H_