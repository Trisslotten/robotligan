#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include "glob/camera.hpp"

struct CameraComponent {
  Camera* cam;
  glm::vec3 offset;
};

#endif //CAMERA_H_