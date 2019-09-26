#ifndef BALL_COMPONENT_HPP_
#define BALL_COMPONENT_HPP_

#include <glm/ext/quaternion_common.hpp>

struct BallComponent {
  bool is_real;
  bool is_airborne;
  glm::quat rotation;
};

#endif  // BALL_COMPONENT_HPP_