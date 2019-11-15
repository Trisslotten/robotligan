#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace glob {

struct Channel {
  char boneID = 0;
  std::vector<aiVectorKey> position_keys;
  std::vector<aiQuatKey> rotation_keys;
  std::vector<aiVectorKey> scaling_keys;
};

class Animation {
 private:
 public:
  std::string name_ = "NO_NAME";
  std::vector<Channel> channels_;
  float tick_per_second_ = 24.f;
  float duration_ = 0.f;
};

}  // namespace glob

#endif  // ANIMATION_HPP_