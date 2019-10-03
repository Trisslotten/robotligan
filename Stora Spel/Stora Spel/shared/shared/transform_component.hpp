#ifndef TRANSFORM_COMPONENT_HPP_
#define TRANSFORM_COMPONENT_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformComponent {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;

  void SetRotation(glm::vec3 rot) {
    rotation = rot;
    //rotation = glm::clamp(rotation, -180.f, 180.f);
  }

  void Rotate(glm::vec3 rot) {
    rotation *= glm::quat(rot);
    //rotation = glm::clamp(rotation, -180.f, 180.f); 
  }

  glm::vec3 Forward() {
    return rotation * glm::vec3(1.f, 0.f, 0.f);  // transform_helper::DirVectorFromRadians(rotation.y,
                                           // rotation.x);
  }
};

#endif  // !TRANSFORM_COMPONENT_H_
