#ifndef ANIMATION_COMPONENT_HPP_
#define ANIMATION_COMPONENT_HPP_
#include "glob/Animation.hpp"
#include "glob/graphics.hpp"
#include "glob/joint.hpp"
#include "glob/playableAnimation.hpp"

struct priorityGroup {
  std::vector<glob::PlayableAnimation*> animations;
  char priority;
};

struct AnimationComponent {
  glob::animData model_data;

  std::vector<glob::PlayableAnimation*> active_animations;

  std::vector<glm::mat4> bone_transforms;

  std::vector<glm::quat> bone_rotations;
  std::vector<glm::mat4> bone_positions;
  std::vector<glm::mat4> bone_scales;

    std::vector<priorityGroup> p_groups;

	bool init = true;

	float yawInterpolator = 0.f;
};

#endif  // ANIMATION_COMPONENT_HPP_


//alternative version
/*
std::vector<playableAnimation> active_animations;

model data{
	innehåller ett bibliotek avpekare till en modells animationer
}

*/