#ifndef FOLLOW_BONE_COMPONENT_HPP_
#define FOLLOW_BONE_COMPONENT_HPP_

#include <vector>
#include <glm/glm.hpp>


struct BoneEmitter {
  int bone_id = 0;
  glm::vec3 pos;
  glm::vec3 dir;
  enum {
    ROCKET,
    HIT,
  } type = ROCKET;

  float speed = 10.f;
};

struct FollowBoneComponent {
  std::vector<BoneEmitter> emitters;
};

#endif // FOLLOW_BONE_COMPONENT_HPP_