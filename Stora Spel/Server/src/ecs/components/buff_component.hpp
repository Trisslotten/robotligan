#ifndef BUFF_COMPONENT_HPP_
#define BUFF_COMPONENT_HPP_

enum BuffID {
  NULL_BUFF,
  SPEED_BOOST,
  JUMP_BOOST,
  INFINITE_STAMINA,
  // Fill with more passive boosts
  NUM_OF_BUFF_IDS
};

struct BuffComponent {
  BuffID active_buff = NULL_BUFF;
  bool buff_toggle = false;
  float duration = 0.0f;
  float duration_remaining = 0.0f;
};

#endif  // !BUFF_COMPONENT_HPP_