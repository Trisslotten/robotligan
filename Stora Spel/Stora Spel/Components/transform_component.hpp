#ifndef TRANSFORM_COMPONENT_HPP_
#define TRANSFORM_COMPONENT_HPP_

#include <glm/glm.hpp>

#include <../util/transform_helper.hpp>

struct TransformComponent {
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  void SetRotation(glm::vec3 rot) {
    rotation = rot;
    //rotation = glm::clamp(rotation, -180.f, 180.f);
  }

  void Rotate(glm::vec3 rot) {
    rotation += rot;
    //rotation = glm::clamp(rotation, -180.f, 180.f); 
  }

  glm::vec3 Forward() {
	return transform_helper::DirVectorFromRadians(rotation.y, rotation.x);
  }
};

#endif  // !TRANSFORM_COMPONENT_H_
