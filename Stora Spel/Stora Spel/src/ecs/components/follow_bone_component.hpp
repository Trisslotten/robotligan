#ifndef FOLLOW_BONE_COMPONENT_HPP_
#define FOLLOW_BONE_COMPONENT_HPP_

#include <vector>
#include <glm/glm.hpp>


enum class BoneEmitterType
{
	ROCKET,
  SLIDE_SPARKS,
	HIT,
  SHOOT,
};

struct BoneEmitter {
  int bone_id = 0;
  glm::vec3 pos;
  glm::vec3 dir;
  BoneEmitterType type = BoneEmitterType::ROCKET;

  float speed = 8.f;

  bool emitted = false;
};

struct FollowBoneComponent {
  std::vector<BoneEmitter> emitters;
};

#endif // FOLLOW_BONE_COMPONENT_HPP_