#ifndef ANIMATION_COMPONENT_HPP_
#define ANIMATION_COMPONENT_HPP_
#include "glob/graphics.hpp"
#include "glob/joint.hpp"
#include "glob/Animation.hpp"

struct AnimationComponent {
	std::vector<glob::Joint> armature;
	std::vector<glob::Animation> animations;

	std::vector<glm::vec4> weights_;
	std::vector<glm::ivec4> bone_index_;

	std::vector<glob::Animation> active_animations;

	std::vector<glm::mat4> bone_transforms;

	void init(glob::ModelHandle model) {
		
	}
};

#endif  // ANIMATION_COMPONENT_HPP_