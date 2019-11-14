#ifndef TRAIL_COMPONENT_HPP_
#define TRAIL_COMPONENT_HPP_

#include <vector>
#include <glm/glm.hpp>

struct TrailHistoryPosition {
  glm::vec3 position;
  float dt = 1.f/30.f;
};

struct TrailComponent {
  float width = 1.5f;
  glm::vec4 color = glm::vec4(0,1,0,1);
  glm::vec3 offset = glm::vec3(0);

  // max 100
  int length = 50;

  std::vector<TrailHistoryPosition> history;
  std::vector<glm::vec3> positions;
  float accum_dt = 0.f;
};

#endif  // TRAIL_COMPONENT_HPP_