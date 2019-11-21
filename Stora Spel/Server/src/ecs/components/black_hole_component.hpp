#ifndef BLACK_HOLE_COMPONENT_HPP_
#define BLACK_HOLE_COMPONENT_HPP_

#include <glm/gtx/quaternion.hpp>
#include "shared/shared.hpp"

struct BlackHoleComponent {
  float time = 1.5f;
  bool is_active = false;
};

#endif  // BLACK_HOLE_COMPONENT_HPP_