#ifndef CAMERA_COMPONENT_HPP_
#define CAMERA_COMPONENT_HPP_

#include <glm/glm.hpp>
#include "../util/transform_helper.hpp"

const float pi = 3.14159265358979323846f;

struct CameraComponent {
  // relative to transform component
  glm::vec3 offset = glm::vec3(0, 0, 0);

  // not relative
  glm::quat orientation;

  glm::vec3 GetLookDir() { return orientation * glm::vec3(1, 0, 0); }
};

#endif  // CAMERA_COMPONENT_HPP_