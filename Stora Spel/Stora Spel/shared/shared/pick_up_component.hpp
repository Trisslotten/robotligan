#ifndef PICK_UP_COMPONENT_HPP_
#define PICK_UP_COMPONENT_HPP_

#include <glm/glm.hpp>

struct PickUpComponent {
  glm::vec3 o_pos;

  bool moving_up = true;
  
  float move_range = 0.4f;

  float current_y_offset = 0;
  float current_rotation_deg = 0;


};

#endif  // !PICK_UP_COMPONENT_HPP_
