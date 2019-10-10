#ifndef ANIMATION_SYSTEM_HPP_
#define ANIMATION_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <entity/utility.hpp>
#include <iostream>

#include "glob/joint.hpp"
#include "glob/Animation.hpp"

#include "ecs/components.hpp"

#include "glm/gtx/matrix_interpolation.hpp"
#include <math.h>

enum ANIM_MODES {LOOP, MUTE_ALL};

glm::mat3 convertToGLM3x3(aiMatrix3x3 aiMat) {
	return {
		  aiMat.a1, aiMat.b1, aiMat.c1,
		  aiMat.a2, aiMat.b2, aiMat.c2,
		  aiMat.a3, aiMat.b3, aiMat.c3
	};
}

void getDefaultPose(glm::mat4 parent, glob::Joint* bone, std::vector<glob::Joint>* armature, glm::mat4 globalInverseTransform) {
	glm::mat4 parentTransform = parent;

	glm::mat4 globalTransform = parentTransform * bone->transform;

	bone->f_transform = globalInverseTransform * globalTransform * bone->offset;

	for (int i = 0; i < bone->children.size(); i++) {
		getDefaultPose(globalTransform, &armature->at(bone->children.at(i)), armature, globalInverseTransform);
	}
}

void playAnimation(int anim, float speed, AnimationComponent *ac, char priority, float strength, int mode, int bodyArgument = -1) {
	for (int i = 0; i < ac->active_animations.size(); i++) {
		if (ac->active_animations.at(i) == &ac->model_data.animations.at(anim)) {
			std::cout << "WARNING: The animation \"" << ac->model_data.animations.at(anim).name_ << "\" is already playing, cannot stack the same animation!\n";
			return;
		}
	}
	glob::Animation* anim_ptr = &ac->model_data.animations.at(anim);
	ac->active_animations.push_back(anim_ptr);
	anim_ptr->speed_ = speed;
	anim_ptr->priority_ = priority;
	anim_ptr->mode_ = mode;
	anim_ptr->playing_ = true;

	if (bodyArgument != -1) {
		anim_ptr->body_argument_ = bodyArgument;
	}

}

float time_ = 0;
bool p = false;

bool isAChildOf(int parent, int lookFor, AnimationComponent* ac) {
		for (int j = 0; j < ac->model_data.bones.at(parent).children.size(); j++) {
			if (ac->model_data.bones.at(parent).children.at(j) == lookFor) {
				return true;
			}
			bool ret = isAChildOf(ac->model_data.bones.at(parent).children.at(j), lookFor, ac);
			if (ret) {
				return true;
			}
		}
	return false;
}
bool first = true;
void UpdateAnimations(entt::registry& registry, float dt) {

	auto animation_entities = registry.view<AnimationComponent>();
	for (auto& entity : animation_entities) {
		auto& a = animation_entities.get(entity);
		int rootBone = a.model_data.armatureRoot;

		std::vector<glm::mat4> boneTransforms;
		std::vector<int> bonePriorities;
		for (int i = 0; i < a.model_data.bones.size(); i++) {
			boneTransforms.push_back(glm::mat4(1.f));
			bonePriorities.push_back(1);
		}

		
		if (first) {
			//playAnimation(11, 2.f, &a, 10, 1.f, LOOP);
			//playAnimation(13, 2.f, &a, 10, 1.f, LOOP);
			playAnimation(7, 1.f, &a, 10, 1.f, LOOP);
			//playAnimation(4, 1.f, &a, 11, 1.f, LOOP, a.model_data.upperBody);
			//playAnimation(11, 2.f, &a, 10, 1.f, LOOP);
			//playAnimation(13, 2.f, &a, 10, 1.f, LOOP);
			//a.active_animations.at(1)->strength = 0.5f;
			first = false;
		}
		/*
		time_ += dt;
		a.active_animations.at(0)->strength_ =  1.f - (sin(time_) + 1.f) / 2.f;
		a.active_animations.at(1)->strength_ = (sin(time_) + 1.f) / 2.f;
		//4 = run

		if (time_ > 7 && !p) {
			//playAnimation(9, 1.f, &a, 10, 1.f, LOOP);
			playAnimation(7, 0.2f, &a, 12, 1.f, MUTE_ALL);
			p = true;
		}
		*/

		for (int i = 0; i < a.active_animations.size(); i++) {
			glob::Animation* anim = a.active_animations.at(i);
			bool removedAnimation = false;
			if (anim->current_frame_time_ >= anim->duration_) {
				//Loop for the time being
				if (anim->mode_ == LOOP) {
					anim->current_frame_time_ = 0;
					for (int j = 0; j < anim->channels_.size(); j++) {
						anim->channels_.at(j).current_position_pos = 0;
						anim->channels_.at(j).current_rotation_pos = 0;
						anim->channels_.at(j).current_scaling_pos = 0;
					}
				}
				else if (anim->mode_ == MUTE_ALL) {
					//reset some values...
					a.active_animations.at(i)->body_argument_ = -1;
					a.active_animations.at(i)->current_frame_time_ = 0.f;
					a.active_animations.at(i)->playing_ = false;
					//remove from list
					a.active_animations.erase(a.active_animations.begin() + i);
					i--;
					removedAnimation = true;
				}
			} else {
				anim->current_frame_time_ +=  anim->speed_ * anim->tick_per_second_ * dt;
			}

			if (!removedAnimation) {
				//hell (aka bone rotation update)
				for (int j = 0; j < anim->channels_.size(); j++) {//all channels (bones)
					glob::Channel* channel = &anim->channels_.at(j);
					int jointId = (int)channel->boneID;

					if (jointId != rootBone) {

						int scalingPos = channel->current_scaling_pos;
						aiVectorKey scalingKey = channel->scaling_keys.at(scalingPos);
						glm::mat4 scaling = glm::scale(glm::vec3(scalingKey.mValue.x, scalingKey.mValue.y, scalingKey.mValue.z));

						if (anim->current_frame_time_ >= scalingKey.mTime) {
							channel->current_scaling_pos++;
						}

						float scaleInterpolation = 0.f;

						int rotationPos = channel->current_rotation_pos;
						aiQuatKey rotationKey = channel->rotation_keys.at(rotationPos);
						glm::mat4 rotation = convertToGLM3x3(rotationKey.mValue.GetMatrix());

						if (anim->current_frame_time_ >= rotationKey.mTime) {
							channel->current_rotation_pos++;
						}

						float rotationInterpolation = 0.f;

						int positionPos = channel->current_position_pos;
						aiVectorKey positionKey = channel->position_keys.at(positionPos);
						glm::mat4 position = glm::translate(glm::vec3(positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z));

						if (anim->current_frame_time_ >= positionKey.mTime) {
							channel->current_position_pos++;
						}

						float positionInterpolation = 0.f;

						glm::mat4 combPRS = position * rotation * scaling;

						glm::mat4 finalMat = combPRS * anim->strength_;

						if (anim->mode_ == LOOP) {
							if (anim->priority_ > bonePriorities.at(jointId)) {//priority override
								if (anim->body_argument_ != -1) {
									if (isAChildOf(anim->body_argument_, jointId, &a)) {//body argument success, override
										bonePriorities.at(jointId) = anim->priority_;
										a.model_data.bones.at(jointId).transform = finalMat;
									}
								}
								else { //No body argument, override
									bonePriorities.at(jointId) = anim->priority_;
									a.model_data.bones.at(jointId).transform = finalMat;
								}
							}
							else if (anim->priority_ == bonePriorities.at(jointId)) {//blend
								if (anim->body_argument_ != -1) {
									if (isAChildOf(anim->body_argument_, jointId, &a)) {//body argument success, blend
										a.model_data.bones.at(jointId).transform += finalMat;
									}
								}
								else { //No body argument, blend
									a.model_data.bones.at(jointId).transform += finalMat;
								}
							}
						}
						else if (anim->mode_ == MUTE_ALL) {
							if (anim->priority_ >= bonePriorities.at(jointId)) {//override
								bonePriorities.at(jointId) = anim->priority_;
								a.model_data.bones.at(jointId).transform = finalMat;
							}
						}
					
					}
				}
			}
		}

		getDefaultPose(glm::mat4(1.f), &a.model_data.bones.at(rootBone), &a.model_data.bones, a.model_data.globalInverseTransform);

		for (auto bone : a.model_data.bones) {
			boneTransforms.at(bone.id) = bone.f_transform;
		}
		a.bone_transforms = boneTransforms;

	}
}

#endif  // ANIMATION_SYSTEM_HPP_
