#ifndef ANIMATION_SYSTEM_HPP_
#define ANIMATION_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <entity/utility.hpp>
#include <iostream>

#include "glob/joint.hpp"
#include "glob/Animation.hpp"

#include "animation_component.hpp"
#include "player_component.hpp"

void UpdateAnimations(entt::registry& registry, float dt) {

	auto animation_entities = registry.view<AnimationComponent>();
	for (auto& entity : animation_entities) {
		auto& a = animation_entities.get(entity);

		std::vector<glm::mat4> boneTransforms;
		for (int i = 0; i < a.model_data.bones.size(); i++) {
			boneTransforms.push_back(glm::mat4(1.f));
		}

		//std::cout << a.model_data.bones.size() << "\n";
		for (auto bone : a.model_data.bones) {
			/*for (int i = 0; i < 16; i++) {
				std::cout << bone.transform[(i / 4) % 4][i % 4] << " : ";
			}*/
			int id = bone.id;
			boneTransforms.at(id) = bone.transform;
		}
		a.bone_transforms = boneTransforms;

		for (int i = 0; i < a.active_animations.size(); i++) {
			/*if (a.active_animations.at(i).current_frame_time_ >= a.active_animations.at(i).duration_) {
				a.active_animations.erase(a.active_animations.begin() + i);
				i--;//think this works?
			} else {*/
				a.active_animations.at(i).current_frame_time_ = a.active_animations.at(i).tick_per_second_ * dt;
			//} //re-implement later

			//hell (aka bone rotation update)

		}
	}
}

#endif  // ANIMATION_SYSTEM_HPP_
