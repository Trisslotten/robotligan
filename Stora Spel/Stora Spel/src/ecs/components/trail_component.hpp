#ifndef TRAIL_COMPONENT_HPP_
#define TRAIL_COMPONENT_HPP_

#include <vector>
#include <glm/glm.hpp>

struct TrailComponent {
  float width = 1.5f;
  glm::vec4 color = glm::vec4(0,1,0,1);
  glm::vec3 offset = glm::vec3(0);

  std::vector<glm::vec3> position_history;
};

#endif  // TRAIL_COMPONENT_HPP_