#ifndef FIREWORKS_COMPONENT_HPP_
#define FIREWORKS_COMPONENT_HPP_

#include <vector>
#include <glm/glm.hpp>

struct FireworksComponent {
  std::vector<glm::vec4> colors = {glm::vec4(1.f)};
  glm::vec3 position = glm::vec3(0.f);
  float spawn = 1.0f;
  float timer = 0.0f;
};

#endif  // FIREWORKS_COMPONENT_HPP_