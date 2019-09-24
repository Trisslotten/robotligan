#ifndef TRANSFORM_HELPER_HPP_
#define TRANSFORM_HELPER_HPP_

#include <glm/glm.hpp>

namespace transform_helper {
inline glm::vec3 DirVectorFromRadians(float yaw, float pitch) {
  float x = cos(yaw) * cos(pitch);
  float y = sin(pitch);
  float z = sin(yaw) * cos(pitch);

  return glm::normalize(glm::vec3(x, y, z));
}
}  // namespace transform_helper

#endif  // TRANSFORM_HELPER_HPP_
