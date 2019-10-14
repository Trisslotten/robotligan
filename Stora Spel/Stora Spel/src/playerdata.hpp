#ifndef PLAYERDATA_HPP_
#define PLAYERDATA_HPP_

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct PlayerData {
  glm::vec3 velocity;
  glm::vec3 delta_pos;
  glm::quat rotation;
  float delta_time;
  std::vector<int> actions;
};

struct FrameState {
  glm::vec3 velocity = glm::vec3(0.f);
  glm::vec3 position = glm::vec3(0.f);
  glm::quat rotation = glm::quat();
};
#endif //PLAYERDATA_HPP_
