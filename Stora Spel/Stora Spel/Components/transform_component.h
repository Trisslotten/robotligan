#ifndef TRANSFORM_COMPONENT_H_
#define TRANSFORM_COMPONENT_H_

#include <glm/glm.hpp>

struct TransformComponent {
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  void setRotation(glm::vec3 rot) {
    rotation = rot;
    //rotation = glm::clamp(rotation, -180.f, 180.f);
  }

  void rotate(glm::vec3 rot) {
    rotation += rot;
    //rotation = glm::clamp(rotation, -180.f, 180.f); 
  }

  glm::vec3 Forward() { //might be broken? works for x and z.
    //sin(rotation.x);

	float x = cos(rotation.y) * cos(rotation.x);
    float y = sin(rotation.x);
    float z = sin(rotation.y) * cos(rotation.x);

    return glm::normalize(glm::vec3(x, y, z));
  }
};

#endif  // !TRANSFORM_COMPONENT_H_
