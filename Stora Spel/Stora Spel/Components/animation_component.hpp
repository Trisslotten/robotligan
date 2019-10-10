#ifndef ANIMATION_COMPONENT_HPP_
#define ANIMATION_COMPONENT_HPP_
#include "glob/graphics.hpp"
#include "glob/joint.hpp"
#include "glob/Animation.hpp"

struct AnimationComponent {
	glob::animData model_data;

	std::vector<glob::Animation*> active_animations;

	std::vector<glm::mat4> bone_transforms;
};

#endif  // ANIMATION_COMPONENT_HPP_