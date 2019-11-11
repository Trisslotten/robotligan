#ifndef PICK_UP_COMPONENT_HPP_
#define PICK_UP_COMPONENT_HPP_

#include <glm/glm.hpp>

struct PickUpComponent {
  glm::vec3 o_pos;
  float move_range = 0.4f;
  bool moving_up = true;
};

#endif  // !PICK_UP_COMPONENT_HPP_
