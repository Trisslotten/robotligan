#ifndef PLAYERDATA_HPP_
#define PLAYERDATA_HPP_

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct PlayerData {
  std::vector<int> actions;
  glm::quat delta_rotation;
  float delta_time;
  float pitch;
  float yaw;
  int id = 0;
};

struct FrameState {
  glm::vec3 velocity = glm::vec3(0.f);
  glm::vec3 position = glm::vec3(10.f, 0.0f, 0.0f);
  float pitch;
  float yaw;
  bool is_airborne = false;
};
#endif //PLAYERDATA_HPP_
