#ifndef ANIMATION_COMPONENT_HPP_
#define ANIMATION_COMPONENT_HPP_
#include "glob/Animation.hpp"
#include "glob/graphics.hpp"
#include "glob/joint.hpp"

struct priorityGroup {
  std::vector<glob::Animation*> animations;
  char priority;
};

struct AnimationComponent {
  glob::animData model_data;

  std::vector<glob::Animation*> active_animations;

  std::vector<glm::mat4> bone_transforms;

    std::vector<priorityGroup> p_groups;
};

#endif  // ANIMATION_COMPONENT_HPP_