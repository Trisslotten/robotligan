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

inline bool InsideBounds2D(glm::vec2 pos_varaible, glm::vec2 pos_entity,
                    glm::vec2 bounds) {
  if (pos_varaible.x >= pos_entity.x &&
      pos_varaible.x <= pos_entity.x + bounds.x) {
    if (pos_varaible.y >= pos_entity.y &&
        pos_varaible.y <= pos_entity.y + bounds.y) {
      return true;
    }
  }
  return false;
}

}  // namespace transform_helper

#endif  // TRANSFORM_HELPER_HPP_
