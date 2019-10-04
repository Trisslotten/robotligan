#ifndef ANIMATION_SYSTEM_HPP_
#define ANIMATION_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <entity/utility.hpp>
#include <iostream>

#include "glob/joint.hpp"
#include "glob/Animation.hpp"

#include "animation_component.hpp"
#include "player_component.hpp"

void getDefaultPose(glm::mat4 parent, glob::Joint* bone, std::vector<glob::Joint>* armature, glm::mat4 GIT) {
	glm::mat4 parentTransform = parent;

	glm::mat4 globalTransform = parentTransform * bone->transform;

	bone->f_transform = GIT * globalTransform * bone->offset;

	for (int i = 0; i < bone->children.size(); i++) {
		getDefaultPose(globalTransform, &armature->at(bone->children.at(i)), armature, GIT);//bone transform is zero (second to last run)
	}
}


void UpdateAnimations(entt::registry& registry, float dt) {

	auto animation_entities = registry.view<AnimationComponent>();
	for (auto& entity : animation_entities) {
		auto& a = animation_entities.get(entity);

		std::vector<glm::mat4> boneTransforms;
		for (int i = 0; i < a.model_data.bones.size(); i++) {
			boneTransforms.push_back(glm::mat4(1.f));
		}

		glm::mat4 identity = glm::mat4(1.f);
		int rootBone = 0;
		for (int i = 0; i < a.model_data.bones.size(); i++) {
			if (a.model_data.bones.at(i).name == "Armature") {
				rootBone = i;
				break;
			}
		}
		getDefaultPose(identity, &a.model_data.bones.at(rootBone), &a.model_data.bones, a.model_data.globalInverseTransform);

		//std::cout << a.model_data.bones.size() << "\n";
		for (auto bone : a.model_data.bones) {
			int id = bone.id;
			boneTransforms.at(id) = bone.f_transform;
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
