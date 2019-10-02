#ifndef MODEL_COMPONENT_HPP_
#define MODEL_COMPONENT_HPP_

#include "glob/graphics.hpp"

struct ModelComponent {
  glob::ModelHandle handle;
  glm::vec3 offset = glm::vec3(0.f);
};

#endif  // MODEL_COMPONENT_HPP_
