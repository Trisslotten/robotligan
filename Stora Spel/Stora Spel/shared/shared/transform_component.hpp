#ifndef TRANSFORM_COMPONENT_HPP_
#define TRANSFORM_COMPONENT_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformComponent {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;

  void SetRotation(glm::vec3 rot) { rotation = rot; }

  void Rotate(glm::vec3 rot) { rotation *= glm::quat(rot); }

  glm::vec3 Forward() {
    return rotation *
           glm::vec3(
               1.f, 0.f,
               0.f);  // transform_helper::DirVectorFromRadians(rotation.y,
                      // rotation.x);
  }

  // Comparasion Operators
  bool operator==(const TransformComponent& rhs) {
    if (this->position != rhs.position) {
      return false;
    }
    if (this->rotation != rhs.rotation) {
      return false;
    }
    if (this->scale != rhs.scale) {
      return false;
    }
    return true;
  }

  bool operator!=(const TransformComponent& rhs) { return !((*this) == rhs); }
};

#endif  // !TRANSFORM_COMPONENT_H_
